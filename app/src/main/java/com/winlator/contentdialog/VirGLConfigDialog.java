package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.CheckBox;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.core.EnvVars;
import com.winlator.core.KeyValueSet;
import com.winlator.core.StringUtils;

import java.util.ArrayList;

public class VirGLConfigDialog extends ContentDialog {
    public static final String DEFAULT_GL_VERSION = "3.1";

    public VirGLConfigDialog(final View anchor) {
        super(anchor.getContext(), R.layout.virgl_config_dialog);
        Context context = anchor.getContext();
        setIcon(R.drawable.icon_settings);
        setTitle("VirGL "+context.getString(R.string.configuration));

        final Spinner sGLVersion = findViewById(R.id.SVersion);
        final CheckBox cbDisableVertexArrayBGRA = findViewById(R.id.CBDisableVertexArrayBGRA);

        KeyValueSet config = new KeyValueSet(anchor.getTag());
        AppUtils.setSpinnerSelectionFromIdentifier(sGLVersion, config.get("glVersion", DEFAULT_GL_VERSION));
        cbDisableVertexArrayBGRA.setChecked(config.getBoolean("disableVertexArrayBGRA", true));

        setOnConfirmCallback(() -> {
            KeyValueSet newConfig = new KeyValueSet();
            newConfig.put("glVersion", StringUtils.parseNumber(sGLVersion.getSelectedItem()));
            newConfig.put("disableVertexArrayBGRA", cbDisableVertexArrayBGRA.isChecked() ? "1" : "0");
            anchor.setTag(newConfig.toString());
        });
    }

    public static void setEnvVars(KeyValueSet config, EnvVars envVars) {
        ArrayList<String> disabledExtensions = new ArrayList<>();
        disabledExtensions.add("GL_KHR_debug");
        if (config.getBoolean("disableVertexArrayBGRA", true)) disabledExtensions.add("GL_EXT_vertex_array_bgra");

        String mesaExtensionOverride = "";
        for (String disabledExtension : disabledExtensions) {
            mesaExtensionOverride += (!mesaExtensionOverride.isEmpty() ? " " : "")+"-"+disabledExtension;
        }

        if (!mesaExtensionOverride.isEmpty()) envVars.put("MESA_EXTENSION_OVERRIDE", mesaExtensionOverride);
        envVars.put("MESA_GL_VERSION_OVERRIDE", config.get("glVersion", DEFAULT_GL_VERSION));
    }
}