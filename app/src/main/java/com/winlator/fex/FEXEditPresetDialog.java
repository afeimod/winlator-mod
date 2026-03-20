package com.winlator.fex;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.ToggleButton;

import androidx.annotation.NonNull;

import com.winlator.R;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.core.AppUtils;
import com.winlator.core.ArrayUtils;
import com.winlator.core.EnvVars;
import com.winlator.core.FileUtils;
import com.winlator.core.StringUtils;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class FEXEditPresetDialog extends ContentDialog {
    private final Context context;
    private final FEXPreset preset;
    private final boolean readonly;
    private Runnable onConfirmCallback;

    public FEXEditPresetDialog(@NonNull Context context, String presetId) {
        super(context, R.layout.box86_64_edit_preset_dialog);
        this.context = context;
        preset = presetId != null ? FEXPresetManager.getPreset(context, presetId) : null;
        readonly = preset != null && !preset.isCustom();
        setTitle("FEX Preset");
        setIcon(R.drawable.icon_env_var);

        final EditText etName = findViewById(R.id.ETName);
        etName.getLayoutParams().width = AppUtils.getPreferredDialogWidth(context);
        etName.setEnabled(!readonly);
        if (preset != null) {
            etName.setText(preset.name);
        }
        else etName.setText(context.getString(R.string.preset)+"-"+FEXPresetManager.getNextPresetId(context));
        loadEnvVarsList();

        super.setOnConfirmCallback(() -> {
            String name = etName.getText().toString().trim();
            if (name.isEmpty()) return;
            name = name.replaceAll("[,\\|]+", "");
            FEXPresetManager.editPreset(context, preset != null ? preset.id : null, name, getEnvVars());
            if (onConfirmCallback != null) onConfirmCallback.run();
        });
    }

    @Override
    public void setOnConfirmCallback(Runnable onConfirmCallback) {
        this.onConfirmCallback = onConfirmCallback;
    }

    private EnvVars getEnvVars() {
        EnvVars envVars = new EnvVars();
        LinearLayout parent = findViewById(R.id.LLContent);
        for (int i = 0; i < parent.getChildCount(); i++) {
            View child = parent.getChildAt(i);
            String name = ((TextView)child.findViewById(R.id.TextView)).getText().toString();

            Spinner spinner = child.findViewById(R.id.Spinner);
            ToggleButton toggleButton = child.findViewById(R.id.ToggleButton);
            EditText editText = child.findViewById(R.id.EditText);

            String value;
            if (toggleButton.getVisibility() == View.VISIBLE) {
                value = toggleButton.isChecked() ? "1" : "0";
            } else if (editText.getVisibility() == View.VISIBLE) {
                value = editText.getText().toString();
            } else {
                value = spinner.getSelectedItem().toString();
            }
            envVars.put(name, value);
        }
        return envVars;
    }

    private void loadEnvVarsList() {
        try {
            LinearLayout parent = findViewById(R.id.LLContent);
            LayoutInflater inflater = LayoutInflater.from(context);
            JSONArray data = new JSONArray(FileUtils.readString(context, "fex_env_vars.json"));
            EnvVars envVars = preset != null ? FEXPresetManager.getEnvVars(context, preset.id) : null;

            for (int i = 0; i < data.length(); i++) {
                JSONObject item = data.getJSONObject(i);
                final String name = item.getString("name");
                View child = inflater.inflate(R.layout.box86_64_env_var_list_item, parent, false);
                ((TextView)child.findViewById(R.id.TextView)).setText(name);

                Spinner spinner = child.findViewById(R.id.Spinner);
                ToggleButton toggleButton = child.findViewById(R.id.ToggleButton);
                EditText editText = child.findViewById(R.id.EditText);
                
                String value = envVars != null && envVars.has(name) ? envVars.get(name) : item.getString("defaultValue");

                if (item.optBoolean("toggleSwitch", false)) {
                    toggleButton.setVisibility(View.VISIBLE);
                    toggleButton.setEnabled(!readonly);
                    toggleButton.setChecked(value.equals("1"));
                    if (readonly) toggleButton.setAlpha(0.5f);
                }
                else if (item.optBoolean("editText", false)) {
                    editText.setVisibility(View.VISIBLE);
                    editText.setEnabled(!readonly);
                    editText.setText(value);
                    if (readonly) editText.setAlpha(0.5f);
                }
                else {
                    String[] values = ArrayUtils.toStringArray(item.getJSONArray("values"));
                    spinner.setVisibility(View.VISIBLE);
                    spinner.setEnabled(!readonly);
                    spinner.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, values));
                    AppUtils.setSpinnerSelectionFromValue(spinner, value);
                }

                parent.addView(child);
            }
        }
        catch (JSONException e) {}
    }
}
