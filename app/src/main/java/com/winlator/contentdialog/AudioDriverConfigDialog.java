package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.KeyValueSet;
import com.winlator.widget.SeekBar;

public class AudioDriverConfigDialog extends ContentDialog {
    public static final byte DEFAULT_PERFORMANCE_MODE = 1;
    public static final float DEFAULT_VOLUME = 1.0f;
    public static final byte DEFAULT_LATENCY_MILLIS = 16;

    public AudioDriverConfigDialog(final View anchor) {
        super(anchor.getContext(), R.layout.audio_driver_config_dialog);
        Context context = anchor.getContext();
        setIcon(R.drawable.icon_audio_settings);
        setTitle(context.getString(R.string.audio)+" "+context.getString(R.string.configuration));

        final Spinner sPerformanceMode = findViewById(R.id.SPerformanceMode);
        final SeekBar sbVolume = findViewById(R.id.SBVolume);
        final SeekBar sbLatencyMillis = findViewById(R.id.SBLatencyMillis);

        KeyValueSet config = new KeyValueSet(anchor.getTag());
        sPerformanceMode.setSelection(config.getInt("performanceMode", DEFAULT_PERFORMANCE_MODE));

        sbVolume.setValue(config.getFloat("volume", DEFAULT_VOLUME) * 100);
        sbLatencyMillis.setValue(config.getInt("latencyMillis", DEFAULT_LATENCY_MILLIS));

        setOnConfirmCallback(() -> {
            KeyValueSet newConfig = new KeyValueSet();
            newConfig.put("performanceMode", sPerformanceMode.getSelectedItemPosition());
            newConfig.put("volume", sbVolume.getValue() / 100.0f);
            newConfig.put("latencyMillis", (int)sbLatencyMillis.getValue());
            anchor.setTag(newConfig.toString());
        });
    }
}