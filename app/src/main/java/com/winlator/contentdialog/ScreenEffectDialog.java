package com.winlator.contentdialog;

import android.content.SharedPreferences;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Spinner;

import androidx.preference.PreferenceManager;

import com.winlator.R;
import com.winlator.XServerDisplayActivity;
import com.winlator.core.AppUtils;
import com.winlator.core.KeyValueSet;
import com.winlator.renderer.GLRenderer;
import com.winlator.renderer.effects.CRTEffect;
import com.winlator.renderer.effects.ColorEffect;
import com.winlator.renderer.effects.FXAAEffect;
import com.winlator.widget.SeekBar;

import java.util.ArrayList;
import java.util.LinkedHashSet;

public class ScreenEffectDialog extends ContentDialog {
    private final XServerDisplayActivity activity;
    private final SharedPreferences preferences;
    private final Spinner sProfile;
    private final SeekBar sbBrightness;
    private final SeekBar sbContrast;
    private final SeekBar sbGamma;
    private final CheckBox cbEnableFXAA;
    private final CheckBox cbEnableCRTShader;

    public ScreenEffectDialog(XServerDisplayActivity activity) {
        super(activity, R.layout.screen_effect_dialog);
        this.activity = activity;
        setTitle(R.string.screen_effect);
        setIcon(R.drawable.icon_screen_effect);

        preferences = PreferenceManager.getDefaultSharedPreferences(activity);

        GLRenderer renderer = activity.getXServerView().getRenderer();
        ColorEffect currentColorEffect = renderer.effectComposer.getEffect(ColorEffect.class);
        final ColorEffect colorEffect = currentColorEffect != null ? currentColorEffect : new ColorEffect();
        final FXAAEffect fxaaEffect = renderer.effectComposer.getEffect(FXAAEffect.class);
        final CRTEffect crtEffect = renderer.effectComposer.getEffect(CRTEffect.class);

        sProfile = findViewById(R.id.SProfile);
        sbBrightness = findViewById(R.id.SBBrightness);
        sbContrast = findViewById(R.id.SBContrast);
        sbGamma = findViewById(R.id.SBGamma);
        cbEnableFXAA = findViewById(R.id.CBEnableFXAA);
        cbEnableCRTShader = findViewById(R.id.CBEnableCRTShader);

        sbBrightness.setValue(colorEffect.getBrightness() * 100);
        sbContrast.setValue(colorEffect.getContrast() * 100);
        sbGamma.setValue(colorEffect.getGamma());
        cbEnableFXAA.setChecked(fxaaEffect != null);
        cbEnableCRTShader.setChecked(crtEffect != null);

        sProfile.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String selectedProfile = position > 0 ? sProfile.getItemAtPosition(position).toString() : null;
                if (selectedProfile != null) loadProfile(selectedProfile);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        loadProfileSpinner(sProfile, activity.getScreenEffectProfile());
        findViewById(R.id.BTAddProfile).setOnClickListener((v) -> ContentDialog.prompt(activity, R.string.profile_name, null, (name) -> {
            addProfile(name, sProfile);
        }));

        findViewById(R.id.BTRemoveProfile).setOnClickListener((v) -> {
            String selectedProfile = sProfile.getSelectedItemPosition() > 0 ? sProfile.getSelectedItem().toString() : null;
            if (selectedProfile != null) {
                ContentDialog.confirm(activity, R.string.do_you_want_to_remove_this_profile, () -> {
                    removeProfile(selectedProfile, sProfile);
                });
            }
            else AppUtils.showToast(activity, R.string.no_profile_selected);
        });

        setOnConfirmCallback(() -> {
            float brightness = sbBrightness.getValue();
            float contrast = sbContrast.getValue();
            float gamma = sbGamma.getValue();

            if (brightness != 0 || contrast != 0 || gamma != 1.0f) {
                colorEffect.setBrightness(brightness / 100.0f);
                colorEffect.setContrast(contrast / 100.0f);
                colorEffect.setGamma(gamma);

                renderer.effectComposer.addEffect(colorEffect);
            }
            else renderer.effectComposer.removeEffect(colorEffect);

            if (cbEnableFXAA.isChecked()) {
                if (fxaaEffect == null) renderer.effectComposer.addEffect(new FXAAEffect());
            }
            else if (fxaaEffect != null) renderer.effectComposer.removeEffect(fxaaEffect);

            if (cbEnableCRTShader.isChecked()) {
                if (crtEffect == null) renderer.effectComposer.addEffect(new CRTEffect());
            }
            else if (crtEffect != null) renderer.effectComposer.removeEffect(crtEffect);

            saveProfile(sProfile);
        });

        Button resetButton = findViewById(R.id.BTReset);
        resetButton.setVisibility(View.VISIBLE);
        resetButton.setOnClickListener((v) -> resetSettings());
    }

    private void resetSettings() {
        sbBrightness.setValue(0);
        sbContrast.setValue(0);
        sbGamma.setValue(1.0f);

        cbEnableFXAA.setChecked(false);
        cbEnableCRTShader.setChecked(false);
    }

    private void saveProfile(Spinner sProfile) {
        String selectedProfile = sProfile.getSelectedItemPosition() > 0 ? sProfile.getSelectedItem().toString() : null;
        if (selectedProfile != null) {
            LinkedHashSet<String> oldProfiles = new LinkedHashSet<>(preferences.getStringSet("screen_effect_profiles", new LinkedHashSet<>()));
            LinkedHashSet<String> newProfiles = new LinkedHashSet<>();

            KeyValueSet settings = new KeyValueSet();
            settings.put("brightness", sbBrightness.getValue());
            settings.put("contrast", sbContrast.getValue());
            settings.put("gamma", sbGamma.getValue());
            settings.put("fxaa", cbEnableFXAA.isChecked());
            settings.put("crt_shader", cbEnableCRTShader.isChecked());

            for (String profile : oldProfiles) {
                String name = profile.split(":")[0];
                newProfiles.add(name.equals(selectedProfile) ? selectedProfile+":"+settings : profile);
            }

            preferences.edit().putStringSet("screen_effect_profiles", newProfiles).apply();
        }

        activity.setScreenEffectProfile(selectedProfile);
    }

    private void loadProfile(String name) {
        LinkedHashSet<String> profiles = new LinkedHashSet<>(preferences.getStringSet("screen_effect_profiles", new LinkedHashSet<>()));
        for (String profile : profiles) {
            String[] parts = profile.split(":");
            if (parts[0].equals(name)) {
                if (parts.length > 1 && !parts[1].isEmpty()) {
                    KeyValueSet settings = new KeyValueSet(parts[1]);

                    sbBrightness.setValue(settings.getFloat("brightness", 0.0f));
                    sbContrast.setValue(settings.getFloat("contrast", 1.0f));
                    sbGamma.setValue(settings.getFloat("gamma", 1.0f));
                    cbEnableFXAA.setChecked(settings.getBoolean("fxaa", false));
                    cbEnableCRTShader.setChecked(settings.getBoolean("crt_shader", false));
                }
                break;
            }
        }
    }

    private void addProfile(String newName, Spinner sProfile) {
        LinkedHashSet<String> profiles = new LinkedHashSet<>(preferences.getStringSet("screen_effect_profiles", new LinkedHashSet<>()));
        for (String profile : profiles) {
            String name = profile.split(":")[0];
            if (name.equals(newName)) return;
        }
        profiles.add(newName.replace(":", "")+":");
        preferences.edit().putStringSet("screen_effect_profiles", profiles).apply();
        loadProfileSpinner(sProfile, newName);
    }

    private void removeProfile(String targetName, Spinner sProfile) {
        LinkedHashSet<String> profiles = new LinkedHashSet<>(preferences.getStringSet("screen_effect_profiles", new LinkedHashSet<>()));
        for (String profile : profiles) {
            String name = profile.split(":")[0];
            if (name.equals(targetName)) {
                profiles.remove(profile);
                break;
            }
        }
        preferences.edit().putStringSet("screen_effect_profiles", profiles).apply();
        loadProfileSpinner(sProfile, null);
        resetSettings();
    }

    private void loadProfileSpinner(Spinner sProfile, String selectedName) {
        LinkedHashSet<String> profiles = new LinkedHashSet<>(preferences.getStringSet("screen_effect_profiles", new LinkedHashSet<>()));
        ArrayList<String> items = new ArrayList<>();

        items.add("-- "+activity.getString(R.string.select_profile)+" --");
        int selectedPosition = 0;
        int position = 1;
        for (String profile : profiles) {
            String name = profile.split(":")[0];
            items.add(name);
            if (name.equals(selectedName)) selectedPosition = position;
            position++;
        }

        sProfile.setAdapter(new ArrayAdapter<>(activity, android.R.layout.simple_spinner_dropdown_item, items));
        sProfile.setSelection(selectedPosition);
    }
}
