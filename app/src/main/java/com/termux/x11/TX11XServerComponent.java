package com.termux.x11;

import static android.content.Context.RECEIVER_NOT_EXPORTED;
import static android.os.Build.VERSION.SDK_INT;
import static android.os.Build.VERSION_CODES.TIRAMISU;
import static com.termux.x11.CmdEntryPoint.ACTION_START;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import com.winlator.xenvironment.EnvironmentComponent;

import java.util.concurrent.CountDownLatch;

/**
 * 作为 XServerComponent 的替代
 */
public class TX11XServerComponent extends EnvironmentComponent {
    private final MainActivity activity;
    private final BroadcastReceiver receiver;
    private final CountDownLatch latch = new CountDownLatch(1);

    public TX11XServerComponent(MainActivity activity) {
        this.activity = activity;
        receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                activity.onReceiveConnection(intent);
                latch.countDown();
            }
        };
    }

    @Override
    @SuppressLint("UnspecifiedRegisterReceiverFlag")
    public void start() {
        // 启动 service 后，CmdEntryPoint 会将自身作为 binder 放入 Intent,发送 ACTION_START 的广播
        activity.registerReceiver(receiver, new IntentFilter(ACTION_START), SDK_INT >= TIRAMISU ? RECEIVER_NOT_EXPORTED : 0);

        // 启动 service (x server)
        activity.startService(new Intent(activity, TX11Service.class));

        // 等待广播
        try {
            latch.await();
            activity.closePreloadDialog();
        } catch (InterruptedException e) {
//            Thread.currentThread().interrupt();
            Log.e("TX11XServerComponent", "等待 tx11 连接时被打断", e);
        } finally {
            activity.unregisterReceiver(receiver);
        }
    }

    @Override
    public void stop() {
        activity.stopService(new Intent(activity, TX11Service.class));
    }
}
