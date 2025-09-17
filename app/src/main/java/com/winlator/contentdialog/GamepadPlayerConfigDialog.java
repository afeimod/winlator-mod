package com.winlator.contentdialog;

import android.content.Context;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.RadioGroup;
import android.widget.Spinner;

import com.winlator.R;
import com.winlator.core.AppUtils;
import com.winlator.core.KeyValueSet;
import com.winlator.inputcontrols.ControlsProfile;
import com.winlator.inputcontrols.ExternalController;
import com.winlator.inputcontrols.InputControlsManager;
import com.winlator.winhandler.GamepadPlayerConfig;

import java.util.ArrayList;

public class GamepadPlayerConfigDialog extends ContentDialog {
    private InputControlsManager manager;

    public GamepadPlayerConfigDialog(final View anchor, byte number) {
        super(anchor.getContext(), R.layout.gamepad_player_config_dialog);
        setIcon(R.drawable.icon_gamepad);
        Context context = anchor.getContext();
        setTitle(context.getString(R.string.player)+" #"+(number+1));

        KeyValueSet config = new KeyValueSet(anchor.getTag());

        final Spinner sSource = findViewById(R.id.SSource);
        byte mode = (byte)config.getInt("mode");
        final String selectedValue = config.get("name");
        final RadioGroup rgMode = findViewById(R.id.RGMode);
        rgMode.check(mode == GamepadPlayerConfig.MODE_EXTERNAL_CONTROLLER ? R.id.RBExternalController : R.id.RBControlsProfile);
        rgMode.setOnCheckedChangeListener((radioGroup, checkedId) -> {
            loadSourceSpinner((byte)(checkedId == R.id.RBExternalController ? GamepadPlayerConfig.MODE_EXTERNAL_CONTROLLER : GamepadPlayerConfig.MODE_CONTROLS_PROFILE), sSource, selectedValue);
        });
        loadSourceSpinner(mode, sSource, selectedValue);

        final CheckBox cbEnableVibration = findViewById(R.id.CBEnableVibration);
        cbEnableVibration.setChecked(config.getBoolean("vibration"));

        setOnConfirmCallback(() -> {
            KeyValueSet newConfig = new KeyValueSet();
            newConfig.put("mode", rgMode.getCheckedRadioButtonId() == R.id.RBExternalController ? GamepadPlayerConfig.MODE_EXTERNAL_CONTROLLER : GamepadPlayerConfig.MODE_CONTROLS_PROFILE);
            newConfig.put("name", sSource.getSelectedItemPosition() > 0 ? sSource.getSelectedItem().toString() : "");
            newConfig.put("vibration", cbEnableVibration.isChecked() ? "1" : "0");
            anchor.setTag(newConfig.toString());
        });
    }

    private void loadSourceSpinner(byte mode, Spinner spinner, String selectedValue) {
        Context context = spinner.getContext();
        ArrayList<String> items = new ArrayList<>();
        items.add(context.getString(R.string.auto));

        if (mode == GamepadPlayerConfig.MODE_EXTERNAL_CONTROLLER) {
            ArrayList<ExternalController> controllers = ExternalController.getControllers();
            for (ExternalController controller : controllers) items.add(controller.getName());
        }
        else {
            if (manager == null) manager = new InputControlsManager(context);
            final ArrayList<ControlsProfile> profiles = manager.getProfiles();
            for (ControlsProfile profile : profiles) {
                profile.loadElements(null);
                if (profile.isVirtualGamepad()) items.add(profile.getName());
            }
        }

        if (!selectedValue.isEmpty() && !items.contains(selectedValue)) items.add(selectedValue);
        spinner.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, items));
        AppUtils.setSpinnerSelectionFromValue(spinner, selectedValue);
    }
}
