package com.termux.x11;

import static android.content.pm.ApplicationInfo.FLAG_TEST_ONLY;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.os.Looper;
import android.os.Process;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;

import androidx.core.app.NotificationCompat;

import com.winlator.R;
import com.winlator.core.ProcessHelper;
import com.winlator.xenvironment.ImageFs;

import java.io.File;

// TODO rootfs 缺少 xkb
public class TX11Service extends Service {
    private static final String TAG = "X11Service";
    public static TX11Service instance = null;
    private boolean started = false;
    private Thread thread = null;
    public TX11Service() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        //显示前台通知，防止切后台后，service被杀
        configureAsForegroundService();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        super.onStartCommand(intent, flags, startId);
        Log.d(TAG, "onStartCommand: 被调用");
        instance = this;
        // service中如果要获取设置，最好只从传过来的intent中读取数据。其他位置的可能不可靠
        if (!started) {
            ImageFs imageFs = ImageFs.find(this);
            File rootDir = imageFs.getRootDir();
            File xkbDir = new File(rootDir, "usr/share/X11/xkb");
            File tmpDir = new File(rootDir, "tmp");
            if (!xkbDir.exists() || !tmpDir.exists()) {
                Log.e(TAG, "onStartCommand: 缺少必要文件夹(xkb或tmp)，不启动xserver。");
                return START_NOT_STICKY;
            }
//            Os.setenv("TERMUX_X11_DEBUG", "1", true)
            try {
                Os.setenv("TERMUX_X11_OVERRIDE_PACKAGE", getPackageName(), true);
                Os.setenv("TMPDIR", tmpDir.getAbsolutePath(), true);
                Os.setenv("XKB_CONFIG_ROOT", xkbDir.getAbsolutePath(), true);
//                com.termux.x11.MainActivity.HOST_PKG_NAME = getPackageName();
            } catch (ErrnoException e) {
                throw new RuntimeException(e);
            }

            started = true;
            thread = new Thread(() -> {
                Looper.prepare(); //不知为何还要调用prepare()
                CmdEntryPoint.main(new String[]{":0"}); //,"-xstartup", "touch ${Consts.getX11StartedValidateFile(timestamp)}" 不行，-xstartup执行完毕就会退出
                Log.d(TAG, "onStartCommand: x11进程结束。停止service");
                started = false;
            });
            thread.start();
        }
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy");
        if (thread != null)
            thread.interrupt();
        started = false;
        stopForeground(true);
        // 要完全杀死进程否则第二次启动容器无法连接
        Process.killProcess(Process.myPid());
    }
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void configureAsForegroundService() {
//        TrayConfiguration trayConfiguration = ((EnvironmentAware) Globals.getApplicationState()).getEnvironment().trayConfiguration;
        Intent intent = new Intent(this, MainActivity.class);
        NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.createNotificationChannel(new NotificationChannel("notification_channel_id", "ExaGear", NotificationManager.IMPORTANCE_DEFAULT));

        Notification build = new NotificationCompat.Builder(this, "notification_channel_id")
                .setSmallIcon(R.drawable.cursor)
                .setContentText("Termux-x11 X Server for Winlator")
                .setContentTitle("Winlator")
                .setContentIntent(PendingIntent.getActivity(this, 0, intent, 0))
                .build();
        notificationManager.notify(2, build);
        startForeground(2, build);
    }
}