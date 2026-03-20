package com.winlator.fex;

import android.content.Context;
import android.content.SharedPreferences;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;

import androidx.preference.PreferenceManager;

import com.winlator.R;
import com.winlator.core.EnvVars;

import java.util.ArrayList;
import java.util.Iterator;

public abstract class FEXPresetManager {
    public static EnvVars getEnvVars(Context context, String id) {
        EnvVars envVars = new EnvVars();

        // 默认基础变量
        envVars.put("FEX_TSOENABLED", "1");
        envVars.put("FEX_VECTORTSOENABLED", "0");
        envVars.put("FEX_HALFBARRIERTSOENABLED", "1");
        envVars.put("FEX_MEMCPYSETTSOENABLED", "0");
        envVars.put("FEX_X87REDUCEDPRECISION", "0");
        envVars.put("FEX_MULTIBLOCK", "1");
        envVars.put("FEX_MAXINST", "5000");
        envVars.put("FEX_HOSTFEATURES", "off");
        envVars.put("FEX_SMALLTSCSCALE", "1");
        envVars.put("FEX_SMC_CHECKS", "mtrack");
        envVars.put("FEX_VOLATILEMETADATA", "1");
        envVars.put("FEX_MONOHACKS", "1");
        envVars.put("FEX_HIDEHYPERVISORBIT", "0");
        envVars.put("FEX_DISABLEL2CACHE", "0");
        envVars.put("FEX_DYNAMICL1CACHE", "0");

        if (id.equals(FEXPreset.STABILITY)) {
            envVars.put("FEX_TSOENABLED", "1");
            envVars.put("FEX_SMC_CHECKS", "mtrack");
            envVars.put("FEX_MULTIBLOCK", "0");
            envVars.put("FEX_HALFBARRIERTSOENABLED", "1");
        }
        else if (id.equals(FEXPreset.COMPATIBILITY)) {
            envVars.put("FEX_TSOENABLED", "1");
            envVars.put("FEX_SMC_CHECKS", "mtrack");
            envVars.put("FEX_MULTIBLOCK", "1");
        }
        else if (id.equals(FEXPreset.INTERMEDIATE)) {
            envVars.put("FEX_TSOENABLED", "1");
            envVars.put("FEX_SMC_CHECKS", "mtrack");
            envVars.put("FEX_MULTIBLOCK", "1");
            envVars.put("FEX_MONOHACKS", "0");
        }
        else if (id.equals(FEXPreset.PERFORMANCE)) {
            envVars.put("FEX_TSOENABLED", "0");
            envVars.put("FEX_SMC_CHECKS", "none");
            envVars.put("FEX_MULTIBLOCK", "1");
            envVars.put("FEX_HALFBARRIERTSOENABLED", "0");
            envVars.put("FEX_VOLATILEMETADATA", "0");
        }
        else if (id.startsWith(FEXPreset.CUSTOM)) {
            for (String[] preset : customPresetsIterator(context)) {
                if (preset[0].equals(id)) {
                    envVars.putAll(preset[2]);
                    break;
                }
            }
        }

        return envVars;
    }

    public static ArrayList<FEXPreset> getPresets(Context context) {
        ArrayList<FEXPreset> presets = new ArrayList<>();
        presets.add(new FEXPreset(FEXPreset.STABILITY, context.getString(R.string.stability)));
        presets.add(new FEXPreset(FEXPreset.COMPATIBILITY, context.getString(R.string.compatibility)));
        presets.add(new FEXPreset(FEXPreset.INTERMEDIATE, context.getString(R.string.intermediate)));
        presets.add(new FEXPreset(FEXPreset.PERFORMANCE, context.getString(R.string.performance)));
        for (String[] preset : customPresetsIterator(context)) presets.add(new FEXPreset(preset[0], preset[1]));
        return presets;
    }

    public static FEXPreset getPreset(Context context, String id) {
        for (FEXPreset preset : getPresets(context)) if (preset.id.equals(id)) return preset;
        return null;
    }

    private static Iterable<String[]> customPresetsIterator(Context context) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        final String customPresetsStr = preferences.getString("fex_custom_presets", "");
        final String[] customPresets = customPresetsStr.split(",");
        final int[] index = {0};
        return () -> new Iterator<String[]>() {
            @Override
            public boolean hasNext() {
                return index[0] < customPresets.length && !customPresetsStr.isEmpty();
            }

            @Override
            public String[] next() {
                return customPresets[index[0]++].split("\\|");
            }
        };
    }

    public static int getNextPresetId(Context context) {
        int maxId = 0;
        for (String[] preset : customPresetsIterator(context)) {
            maxId = Math.max(maxId, Integer.parseInt(preset[0].replace(FEXPreset.CUSTOM+"-", "")));
        }
        return maxId+1;
    }

    public static void editPreset(Context context, String id, String name, EnvVars envVars) {
        String key = "fex_custom_presets";
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String customPresetsStr = preferences.getString(key, "");

        if (id != null) {
            String[] customPresets = customPresetsStr.split(",");
            for (int i = 0; i < customPresets.length; i++) {
                String[] preset = customPresets[i].split("\\|");
                if (preset[0].equals(id)) {
                    customPresets[i] = id+"|"+name+"|"+envVars.toString();
                    break;
                }
            }
            customPresetsStr = String.join(",", customPresets);
        }
        else {
            String preset = FEXPreset.CUSTOM+"-"+getNextPresetId(context)+"|"+name+"|"+envVars.toString();
            customPresetsStr += (!customPresetsStr.isEmpty() ? "," : "")+preset;
        }
        preferences.edit().putString(key, customPresetsStr).apply();
    }

    public static void duplicatePreset(Context context, String id) {
        ArrayList<FEXPreset> presets = getPresets(context);
        FEXPreset originPreset = null;
        for (FEXPreset preset : presets) {
            if (preset.id.equals(id)) {
                originPreset = preset;
                break;
            }
        }
        if (originPreset == null) return;

        String newName;
        for (int i = 1;;i++) {
            newName = originPreset.name+" ("+i+")";
            boolean found = false;
            for (FEXPreset preset : presets) {
                if (preset.name.equals(newName)) {
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }

        editPreset(context, null, newName, getEnvVars(context, originPreset.id));
    }

    public static void removePreset(Context context, String id) {
        String key = "fex_custom_presets";
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(context);
        String oldCustomPresetsStr = preferences.getString(key, "");
        String newCustomPresetsStr = "";

        String[] customPresets = oldCustomPresetsStr.split(",");
        for (int i = 0; i < customPresets.length; i++) {
            String[] preset = customPresets[i].split("\\|");
            if (!preset[0].equals(id)) newCustomPresetsStr += (!newCustomPresetsStr.isEmpty() ? "," : "")+customPresets[i];
        }

        preferences.edit().putString(key, newCustomPresetsStr).apply();
    }

    public static void loadSpinner(Spinner spinner, String selectedId) {
        Context context = spinner.getContext();
        ArrayList<FEXPreset> presets = getPresets(context);

        int selectedPosition = 0;
        for (int i = 0; i < presets.size(); i++) {
            if (presets.get(i).id.equals(selectedId)) {
                selectedPosition = i;
                break;
            }
        }

        spinner.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, presets));
        spinner.setSelection(selectedPosition);
    }

    public static String getSpinnerSelectedId(Spinner spinner) {
        SpinnerAdapter adapter = spinner.getAdapter();
        int selectedPosition = spinner.getSelectedItemPosition();
        if (adapter != null && adapter.getCount() > 0 && selectedPosition >= 0) {
            return ((FEXPreset)adapter.getItem(selectedPosition)).id;
        }
        else return FEXPreset.COMPATIBILITY;
    }
}
