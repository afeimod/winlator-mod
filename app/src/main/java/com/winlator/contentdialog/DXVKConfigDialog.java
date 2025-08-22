package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.core.DefaultVersion;
import com.winlator.core.EnvVars;
import com.winlator.core.FileUtils;
import com.winlator.core.GeneralComponents;
import com.winlator.core.KeyValueSet;
import com.winlator.core.StringUtils;
import com.winlator.widget.GPUCardAdapter;
import com.winlator.xenvironment.RootFS;

import java.io.File;

public class DXVKConfigDialog extends ContentDialog {
    public DXVKConfigDialog(String graphicsDriver, final View anchor) {
        super(anchor.getContext(), R.layout.dxvk_config_dialog);
        setIcon(R.drawable.icon_display_settings);
        Context context = anchor.getContext();
        setTitle("DXVK "+context.getString(R.string.configuration));

        final Spinner sVersion = findViewById(R.id.SVersion);
        final Spinner sFramerate = findViewById(R.id.SFramerate);
        final Spinner sMaxDeviceMemory = findViewById(R.id.SMaxDeviceMemory);
        final Spinner sCustomDevice = findViewById(R.id.SCustomDevice);

        KeyValueSet config = new KeyValueSet(anchor.getTag());
        AppUtils.setSpinnerSelectionFromIdentifier(sFramerate, config.get("framerate", "0"));
        AppUtils.setSpinnerSelectionFromNumber(sMaxDeviceMemory, config.get("maxDeviceMemory", "0"));

        String version = config.get("version");
        String defaultVersion = DefaultVersion.DXVK(graphicsDriver);
        GeneralComponents.initViews(GeneralComponents.Type.DXVK, findViewById(R.id.DXVKToolbox), sVersion, version, defaultVersion);

        GPUCardAdapter adapter = new GPUCardAdapter(context, android.R.layout.simple_spinner_dropdown_item, R.string.none);
        sCustomDevice.setAdapter(adapter);

        String customDevice = config.get("customDevice");
        if (customDevice.contains(":")) {
            try {
                int deviceId = Integer.parseInt(customDevice.split(":")[0], 16);
                sCustomDevice.setSelection(adapter.getPositionByDeviceId(deviceId));
            }
            catch (NumberFormatException e) {}
        }

        setOnConfirmCallback(() -> {
            KeyValueSet newConfig = new KeyValueSet();
            newConfig.put("version", StringUtils.parseNumber(sVersion.getSelectedItem()));
            newConfig.put("framerate", StringUtils.parseNumber(sFramerate.getSelectedItem()));
            newConfig.put("maxDeviceMemory", StringUtils.parseNumber(sMaxDeviceMemory.getSelectedItem()));

            GPUCardAdapter.GPUCard gpuCard = (GPUCardAdapter.GPUCard)sCustomDevice.getSelectedItem();
            if (gpuCard.deviceId > 0) {
                newConfig.put("customDevice", String.format("%04x", gpuCard.deviceId)+":"+String.format("%04x", gpuCard.vendorId)+":"+gpuCard.name);
            }

            anchor.setTag(newConfig.toString());
        });
    }

    public static void setEnvVars(Context context, KeyValueSet config, EnvVars envVars) {
        envVars.put("DXVK_STATE_CACHE_PATH", RootFS.getDosUserCachePath());
        envVars.put("DXVK_LOG_LEVEL", "none");

        File rootDir = RootFS.find(context).getRootDir();
        File dxvkConfigFile = new File(rootDir, RootFS.USER_CONFIG_PATH+"/dxvk.conf");

        String content = "";
        String maxDeviceMemory = config.get("maxDeviceMemory");
        if (!maxDeviceMemory.isEmpty() && !maxDeviceMemory.equals("0")) {
            content += "dxgi.maxDeviceMemory = "+maxDeviceMemory+"\n";
            content += "dxgi.maxSharedMemory = "+maxDeviceMemory+"\n";
        }

        String framerate = config.get("framerate");
        if (!framerate.isEmpty() && !framerate.equals("0")) {
            content += "dxgi.maxFrameRate = "+framerate+"\n";
            content += "d3d9.maxFrameRate = "+framerate+"\n";
        }

        String customDevice = config.get("customDevice");
        if (customDevice.contains(":")) {
            String[] parts = customDevice.split(":");
            content += "dxgi.customDeviceId = "+parts[0]+"\n";
            content += "dxgi.customVendorId = "+parts[1]+"\n";

            content += "d3d9.customDeviceId = "+parts[0]+"\n";
            content += "d3d9.customVendorId = "+parts[1]+"\n";

            content += "dxgi.customDeviceDesc = \""+parts[2]+"\"\n";
            content += "d3d9.customDeviceDesc = \""+parts[2]+"\"\n";
        }

        content += "d3d11.constantBufferRangeCheck = \"True\"\n";

        FileUtils.delete(dxvkConfigFile);
        if (FileUtils.writeString(dxvkConfigFile, content)) {
            envVars.put("DXVK_CONFIG_FILE", RootFS.getDosUserConfigPath()+"\\dxvk.conf");
        }
    }
}
