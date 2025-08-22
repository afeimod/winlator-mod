package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.core.DefaultVersion;
import com.winlator.core.EnvVars;
import com.winlator.core.GPUHelper;
import com.winlator.core.GeneralComponents;
import com.winlator.core.KeyValueSet;
import com.winlator.core.StringUtils;

public class TurnipConfigDialog extends ContentDialog {
    private static final int MAX_DEVICE_MEMORY = 4096;

    public TurnipConfigDialog(final View anchor) {
        super(anchor.getContext(), R.layout.turnip_config_dialog);
        Context context = anchor.getContext();
        setIcon(R.drawable.icon_display_settings);
        setTitle("Turnip "+context.getString(R.string.configuration));

        final Spinner sVersion = findViewById(R.id.SVersion);
        final Spinner sMaxDeviceMemory = findViewById(R.id.SMaxDeviceMemory);
        final CheckBox cbUseHWBuf = findViewById(R.id.CBUseHWBuf);
        final CheckBox cbForceWaitForFences = findViewById(R.id.CBForceWaitForFences);

        KeyValueSet config = new KeyValueSet(anchor.getTag());
        cbUseHWBuf.setChecked(config.getBoolean("useHWBuf", true));
        cbForceWaitForFences.setChecked(config.getBoolean("forceWaitForFences"));
        AppUtils.setSpinnerSelectionFromNumber(sMaxDeviceMemory, config.get("maxDeviceMemory", String.valueOf(MAX_DEVICE_MEMORY)));

        String version = config.get("version");
        GeneralComponents.initViews(GeneralComponents.Type.TURNIP, findViewById(R.id.TurnipToolbox), sVersion, version, DefaultVersion.TURNIP);

        setOnConfirmCallback(() -> {
            KeyValueSet newConfig = new KeyValueSet();
            newConfig.put("version", StringUtils.parseNumber(sVersion.getSelectedItem()));
            newConfig.put("maxDeviceMemory", StringUtils.parseNumber(sMaxDeviceMemory.getSelectedItem()));
            newConfig.put("useHWBuf", cbUseHWBuf.isChecked() ? "1" : "0");
            newConfig.put("forceWaitForFences", cbForceWaitForFences.isChecked() ? "1" : "0");
            anchor.setTag(newConfig.toString());
        });
    }

    public static void setEnvVars(Context context, KeyValueSet config, EnvVars envVars) {
        envVars.put("TU_OVERRIDE_HEAP_SIZE", config.get("maxDeviceMemory", String.valueOf(MAX_DEVICE_MEMORY)));
        if (config.getBoolean("useHWBuf", true)) envVars.put("MESA_VK_WSI_USE_HWBUF", "1");
        if (config.getBoolean("forceWaitForFences")) envVars.put("MESA_VK_WSI_FORCE_WAIT_FOR_FENCES", "1");

        String tuDebug = envVars.get("TU_DEBUG");
        if (!GPUHelper.isAdreno6xx(context) && !tuDebug.contains("sysmem")) {
            envVars.put("TU_DEBUG", (!tuDebug.isEmpty() ? tuDebug + "," : "") + "sysmem");
        }
    }
}