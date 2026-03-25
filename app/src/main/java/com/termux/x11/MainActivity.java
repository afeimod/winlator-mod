package com.termux.x11;

import static android.os.Build.VERSION.SDK_INT;
import static android.os.Build.VERSION_CODES.TIRAMISU;
import static com.termux.x11.CmdEntryPoint.ACTION_START;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.PointerIcon;
import android.view.View;
import android.widget.FrameLayout;

//import com.termux.x11.MainActivity;
import androidx.annotation.Keep;
import androidx.annotation.NonNull;

import com.winlator.R;
import com.winlator.XServerDisplayActivity;
import com.winlator.widget.XServerView;
import com.winlator.xserverbridge.IXServerBridge;
import com.winlator.xserverbridge.TX11XServerBridge;

// native 中固定包名类名获取 java 函数，所以这个类不能移动或重命名
public class MainActivity extends XServerDisplayActivity {
    private static final String TAG = "Tx11MainActivity";
    private static MainActivity instance = null;
    private LorieView lorieView = null;
    protected ICmdEntryInterface service = null;
    private BroadcastReceiver receiver = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate: 启动 tx11 activity");
        instance = this;

        lorieView = new LorieView(this);
        lorieView.setCallback((surfaceWidth, surfaceHeight, screenWidth, screenHeight) -> {
            String name;
            int framerate = (int) ((lorieView.getDisplay() != null) ? lorieView.getDisplay().getRefreshRate() : 30);

//            mInputHandler.handleHostSizeChanged(surfaceWidth, surfaceHeight);
//            mInputHandler.handleClientSizeChanged(screenWidth, screenHeight);
            if (lorieView.getDisplay() == null || lorieView.getDisplay().getDisplayId() == Display.DEFAULT_DISPLAY)
                name = "builtin";
            else
                name = "external";
            LorieView.sendWindowChange(screenWidth, screenHeight, framerate, name);
        });

        // 原 XServerDisplayActivity 逻辑
        super.onCreate(savedInstanceState);

        // 添加 LorieView
        FrameLayout rootView = findViewById(R.id.FLXServerDisplay);
        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(-1, -1);
        lp.gravity = Gravity.CENTER;
        rootView.addView(lorieView, 0, lp);
        // 桌面分辨率
        lorieView.p.set(xServer.screenInfo.width, xServer.screenInfo.height);

        // 启动 service 后，CmdEntryPoint 会将自身作为 binder 放入 Intent,发送 ACTION_START 的广播,
        // 在这里接收广播，获取 binder 并尝试连接 x server
        receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (ACTION_START.equals(intent.getAction())) {
                    onReceiveConnection(intent);
                }
            }
        };
        registerReceiver(receiver, new IntentFilter(ACTION_START), SDK_INT >= TIRAMISU ? RECEIVER_NOT_EXPORTED : 0);

        // 启动 service (x server)
        startService(new Intent(this, TX11Service.class));
        preloaderDialog.close();
        // 目前原 xserver 也会启动 (XServerComponent)，不知道不关会不会有影响
    }

    void onReceiveConnection(Intent intent) {
        Bundle bundle = intent == null ? null : intent.getBundleExtra(null);
        IBinder ibinder = bundle == null ? null : bundle.getBinder(null);
        if (ibinder == null)
            return;

        service = ICmdEntryInterface.Stub.asInterface(ibinder);
        try {
            service.asBinder().linkToDeath(() -> {
                service = null;

                Log.v("Lorie", "Disconnected");
                runOnUiThread(() -> { LorieView.connect(-1); clientConnectedStateChanged();} );
            }, 0);
        } catch (RemoteException ignored) {}

        try {
            if (service != null && service.asBinder().isBinderAlive()) {
                Log.v("LorieBroadcastReceiver", "Extracting logcat fd.");
                ParcelFileDescriptor logcatOutput = service.getLogcatOutput();
                if (logcatOutput != null)
                    LorieView.startLogcat(logcatOutput.detachFd());

                tryConnect();

                if (intent != getIntent())
                    getIntent().putExtra(null, bundle);
            }
        } catch (Exception e) {
            Log.e("MainActivity", "Something went wrong while we were establishing connection", e);
        }
    }

    boolean tryConnect() {
        if (LorieView.connected())
            return false;

        if (service == null) {
            boolean sent = LorieView.requestConnection();
            lorieView.postDelayed(this::tryConnect, 250);
            return true;
        }

        try {
            ParcelFileDescriptor fd = service.getXConnection();
            if (fd != null) {
                Log.v("MainActivity", "Extracting X connection socket.");
                LorieView.connect(fd.detachFd());
                getLorieView().triggerCallback();
                clientConnectedStateChanged();
//                getLorieView().reloadPreferences(prefs);
            } else
                lorieView.postDelayed(this::tryConnect, 250);
        } catch (Exception e) {
            Log.e("MainActivity", "Something went wrong while we were establishing connection", e);
            service = null;

            lorieView.postDelayed(this::tryConnect, 250);
        }
        return false;
    }

    @Keep // used in native code
    void clientConnectedStateChanged() {
        runOnUiThread(()-> {
            boolean connected = LorieView.connected();
            getLorieView().setVisibility(connected? View.VISIBLE:View.INVISIBLE);

            // We should recover connection in the case if file descriptor for some reason was broken...
            if (!connected)
                tryConnect();
            else
                getLorieView().setPointerIcon(PointerIcon.getSystemIcon(this, PointerIcon.TYPE_NULL));

            onWindowFocusChanged(hasWindowFocus());
        });
    }

    @Override
    protected IXServerBridge createXServerBridge() {
        return new TX11XServerBridge(lorieView, xServer);
    }

    @Override
    protected void setupUI() {
        super.setupUI();
        // LorieView 放在了最下层，所以要隐藏原 xServerView 防止挡住
        FrameLayout rootView = findViewById(R.id.FLXServerDisplay);
        for (int i = 0; i < rootView.getChildCount(); i ++) {
            if (rootView.getChildAt(i) instanceof XServerView) {
                rootView.getChildAt(i).setVisibility(View.GONE);
            }
        }
    }

    @Override
    protected boolean onXServerKeyboardKeyEvent(KeyEvent event) {
        return KeyEventSender.instance.sendKeyEvent(event, lorieView);
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull MenuItem item) {
        if (item.getItemId() == R.id.main_menu_toggle_fullscreen) {
            lorieView.toggleStretchFullscreen();
        }
        return super.onNavigationItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        unregisterReceiver(receiver);
        stopService(new Intent(this, TX11Service.class));
        instance = null;
    }

    @Keep // used in native code
    public static MainActivity getInstance() {
        return instance;
    }

    public LorieView getLorieView() {
        return lorieView;
    }

    //    @Override
//    protected void onCreate(Bundle savedInstanceState) {
////        MainActivity.HOST_PKG_NAME = getPackageName();
//        super.onCreate(savedInstanceState);
//        // 要不还是继承XServerDisplayActivity 然后 tx11activity的内容移植过来。aar 里有没有用到 MainActivity 的地方？
//        // 运行 xserver
//        startService(new Intent(this, TX11Service.class));
//        // 显示安卓视图
//    }

}
