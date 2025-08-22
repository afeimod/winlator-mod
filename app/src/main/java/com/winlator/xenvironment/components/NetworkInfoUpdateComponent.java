package com.winlator.xenvironment.components;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;

import com.winlator.core.FileUtils;
import com.winlator.core.NetworkHelper;
import com.winlator.xenvironment.EnvironmentComponent;

import java.io.File;
import java.util.List;

public class NetworkInfoUpdateComponent extends EnvironmentComponent {
    private BroadcastReceiver broadcastReceiver;

    @Override
    public void start() {
        Context context = environment.getContext();
        final NetworkHelper networkHelper = new NetworkHelper(context);
        updateIFAddrsFile(networkHelper.getIFAddresses());
        updateEtcHostsFile(networkHelper.getIPv4Address());

        broadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                updateIFAddrsFile(networkHelper.getIFAddresses());
                updateEtcHostsFile(networkHelper.getIPv4Address());
            }
        };

        IntentFilter filter = new IntentFilter();
        filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
        context.registerReceiver(broadcastReceiver, filter);
    }

    @Override
    public void stop() {
        if (broadcastReceiver != null) {
            environment.getContext().unregisterReceiver(broadcastReceiver);
            broadcastReceiver = null;
        }
    }

    private void updateIFAddrsFile(List<NetworkHelper.IFAddress> ifAddresses) {
        File file = new File(environment.getRootFS().getTmpDir(), "ifaddrs");

        String content = "";
        if (!ifAddresses.isEmpty()) {
            for (NetworkHelper.IFAddress ifAddress : ifAddresses) {
                content += (!content.isEmpty() ? "\n" : "")+ifAddress.toString();
            }
        }
        else content = (new NetworkHelper.IFAddress()).toString();

        FileUtils.writeString(file, content);
    }

    private void updateEtcHostsFile(String ipAddress) {
        String ip = ipAddress != null ? ipAddress : "127.0.0.1";
        File file = new File(environment.getRootFS().getRootDir(), "etc/hosts");
        FileUtils.writeString(file, ip+"\tlocalhost\n");
    }
}
