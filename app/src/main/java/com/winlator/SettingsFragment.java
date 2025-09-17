package com.winlator;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.media.midi.MidiDeviceInfo;
import android.media.midi.MidiManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.preference.PreferenceManager;

import com.google.android.material.navigation.NavigationView;
import com.winlator.box64.Box64EditPresetDialog;
import com.winlator.box64.Box64Preset;
import com.winlator.box64.Box64PresetManager;
import com.winlator.container.Container;
import com.winlator.container.ContainerManager;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.contentdialog.GamepadPlayerConfigDialog;
import com.winlator.contentdialog.SoundFontTestDialog;
import com.winlator.core.AppUtils;
import com.winlator.core.ArrayUtils;
import com.winlator.core.Callback;
import com.winlator.core.DefaultVersion;
import com.winlator.core.FileUtils;
import com.winlator.core.GeneralComponents;
import com.winlator.core.LocaleHelper;
import com.winlator.core.PreloaderDialog;
import com.winlator.core.StringUtils;
import com.winlator.core.WineInfo;
import com.winlator.core.WineUtils;
import com.winlator.widget.ColorPickerView;
import com.winlator.widget.LogView;
import com.winlator.widget.SeekBar;
import com.winlator.winhandler.GamepadHandler;
import com.winlator.xenvironment.RootFS;
import com.winlator.xenvironment.RootFSInstaller;

import org.json.JSONArray;
import org.json.JSONException;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicLong;

public class SettingsFragment extends Fragment {
    public static final String DEFAULT_WINE_DEBUG_CHANNELS = "warn,err,fixme";
    public static final byte APP_THEME_LIGHT = 0;
    public static final byte APP_THEME_DARK = 1;
    private Callback<Uri> selectWineFileCallback;
    private PreloaderDialog preloaderDialog;
    private SharedPreferences preferences;
    private boolean midiDeviceCallbackRegistered = false;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(false);
        preloaderDialog = new PreloaderDialog(getActivity());
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        ((AppCompatActivity)getActivity()).getSupportActionBar().setTitle(R.string.settings);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (requestCode == MainActivity.OPEN_FILE_REQUEST_CODE && resultCode == Activity.RESULT_OK) {
            try {
                if (selectWineFileCallback != null && data != null) selectWineFileCallback.call(data.getData());
            }
            catch (Exception e) {
                AppUtils.showToast(getContext(), R.string.unable_to_import_profile);
            }
            selectWineFileCallback = null;
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.settings_fragment, container, false);
        final Context context = getContext();
        preferences = PreferenceManager.getDefaultSharedPreferences(context);

        final Spinner sSoundFont = view.findViewById(R.id.SSoundFont);
        String soundfont = preferences.getString("soundfont", null);
        GeneralComponents.initViews(GeneralComponents.Type.SOUNDFONT, view.findViewById(R.id.SoundFontToolbox), sSoundFont, soundfont, DefaultVersion.SOUNDFONT);
        view.findViewById(R.id.BTSoundFontTest).setOnClickListener((v) -> (new SoundFontTestDialog(context, sSoundFont.getSelectedItem().toString())).show());

        final Spinner sMIDIInputDevice = view.findViewById(R.id.SMIDIInputDevice);
        String midiInputDevice = preferences.getString("midi_input_device", "auto");
        loadMIDIInputDeviceSpinner(sMIDIInputDevice, midiInputDevice);

        final Spinner sBox64Version = view.findViewById(R.id.SBox64Version);
        String box64Version = preferences.getString("box64_version", null);
        GeneralComponents.initViews(GeneralComponents.Type.BOX64, view.findViewById(R.id.Box64Toolbox), sBox64Version, box64Version, DefaultVersion.BOX64);

        final Spinner sBox64Preset = view.findViewById(R.id.SBox64Preset);
        loadBox64PresetSpinner(view, sBox64Preset);

        final RadioGroup rgAppTheme = view.findViewById(R.id.RGAppTheme);
        final int oldAppThemeId = preferences.getInt("app_theme", APP_THEME_DARK) == APP_THEME_DARK ? R.id.RBDark : R.id.RBLight;
        rgAppTheme.check(oldAppThemeId);

        final CheckBox cbMoveCursorToTouchpoint = view.findViewById(R.id.CBMoveCursorToTouchpoint);
        cbMoveCursorToTouchpoint.setChecked(preferences.getBoolean("move_cursor_to_touchpoint", false));

        final CheckBox cbCapturePointerOnExternalMouse = view.findViewById(R.id.CBCapturePointerOnExternalMouse);
        cbCapturePointerOnExternalMouse.setChecked(preferences.getBoolean("capture_pointer_on_external_mouse", true));

        final CheckBox cbOpenAndroidBrowserFromWine = view.findViewById(R.id.CBOpenAndroidBrowserFromWine);
        cbOpenAndroidBrowserFromWine.setChecked(preferences.getBoolean("open_android_browser_from_wine", true));

        final CheckBox cbUseAndroidClipboardOnWine = view.findViewById(R.id.CBUseAndroidClipboardOnWine);
        cbUseAndroidClipboardOnWine.setChecked(preferences.getBoolean("use_android_clipboard_on_wine", false));

        final CheckBox cbEnableWineDebug = view.findViewById(R.id.CBEnableWineDebug);
        cbEnableWineDebug.setChecked(preferences.getBoolean("enable_wine_debug", false));

        final ArrayList<String> wineDebugChannels = new ArrayList<>(Arrays.asList(preferences.getString("wine_debug_channels", DEFAULT_WINE_DEBUG_CHANNELS).split(",")));
        loadWineDebugChannels(view, wineDebugChannels);

        final Spinner sBox64Logs = view.findViewById(R.id.SBox64Logs);
        sBox64Logs.setSelection(preferences.getInt("box64_logs", 0));

        final CheckBox cbSaveLogsToFile = view.findViewById(R.id.CBSaveLogsToFile);
        cbSaveLogsToFile.setChecked(preferences.getBoolean("save_logs_to_file", false));

        final EditText etLogFile = view.findViewById(R.id.ETLogFile);
        final String defaultLogPath = LogView.getLogFile().getPath();
        etLogFile.setText(preferences.getString("log_file", defaultLogPath));
        etLogFile.setVisibility(cbSaveLogsToFile.isChecked() ? View.VISIBLE : View.GONE);
        cbSaveLogsToFile.setOnCheckedChangeListener((buttonView, isChecked) -> etLogFile.setVisibility(isChecked ? View.VISIBLE : View.GONE));

        final SeekBar sbCursorSpeed = view.findViewById(R.id.SBCursorSpeed);
        sbCursorSpeed.setValue(preferences.getFloat("cursor_speed", 1.0f) * 100);

        final SeekBar sbCursorSize = view.findViewById(R.id.SBCursorSize);
        sbCursorSize.setValue(preferences.getFloat("cursor_scale", 1.0f) * 100);

        final ColorPickerView cpvCursorColor = view.findViewById(R.id.CPVCursorColor);
        cpvCursorColor.setPalette(0xffffff, 0x000000, 0x651fff, 0xffea00, 0xff9100, 0xf50057, 0x00b0ff, 0x1de9b6);
        cpvCursorColor.setColor(preferences.getInt("cursor_color", 0xffffff));

        final Spinner sPreferredInputApi = view.findViewById(R.id.SPreferredInputApi);
        sPreferredInputApi.setSelection(preferences.getInt("preferred_input_api", GamepadHandler.PreferredInputApi.AUTO.ordinal()));

        final Spinner sWineVersion = view.findViewById(R.id.SWineVersion);
        loadWineVersionSpinner(view, sWineVersion);

        final Spinner sLanguage = view.findViewById(R.id.SLanguage);
        sLanguage.setSelection(LocaleHelper.getLocaleIndex(context));
        final int oldLCIndex = sLanguage.getSelectedItemPosition();

        view.findViewById(R.id.BTReinstallSystemFiles).setOnClickListener((v) -> {
            ContentDialog.confirm(context, R.string.do_you_want_to_reinstall_system_files, () -> RootFSInstaller.install((MainActivity)getActivity()));
        });

        loadGamepadPlayerConfigs(view);

        if (MainActivity.DEBUG_MODE) {
            view.findViewById(R.id.LLWineInstallation).setVisibility(View.VISIBLE);
        }

        view.findViewById(R.id.BTConfirm).setOnClickListener((v) -> {
            SharedPreferences.Editor editor = preferences.edit();
            editor.putString("soundfont", sSoundFont.getSelectedItem().toString());
            editor.putString("box64_version", StringUtils.parseIdentifier(sBox64Version.getSelectedItem()));
            editor.putString("box64_preset", Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            editor.putBoolean("move_cursor_to_touchpoint", cbMoveCursorToTouchpoint.isChecked());
            editor.putBoolean("capture_pointer_on_external_mouse", cbCapturePointerOnExternalMouse.isChecked());
            editor.putFloat("cursor_speed", sbCursorSpeed.getValue() / 100.0f);
            editor.putFloat("cursor_scale", sbCursorSize.getValue() / 100.0f);
            editor.putInt("cursor_color", cpvCursorColor.getColor());
            editor.putBoolean("enable_wine_debug", cbEnableWineDebug.isChecked());
            editor.putInt("box64_logs", sBox64Logs.getSelectedItemPosition());
            editor.putBoolean("save_logs_to_file", cbSaveLogsToFile.isChecked());
            editor.putInt("preferred_input_api", sPreferredInputApi.getSelectedItemPosition());
            editor.putBoolean("open_android_browser_from_wine", cbOpenAndroidBrowserFromWine.isChecked());
            editor.putBoolean("use_android_clipboard_on_wine", cbUseAndroidClipboardOnWine.isChecked());
            putGamepadPlayerConfigs(view, editor);

            int newAppThemeId = rgAppTheme.getCheckedRadioButtonId();
            editor.putInt("app_theme", newAppThemeId == R.id.RBLight ? APP_THEME_LIGHT : APP_THEME_DARK);

            int newLCIndex = sLanguage.getSelectedItemPosition();
            editor.putInt("lc_index", newLCIndex);
            boolean restartApp = oldLCIndex != newLCIndex || oldAppThemeId != newAppThemeId;

            int midiInputDevicePosition = sMIDIInputDevice.getSelectedItemPosition();
            editor.putString("midi_input_device", midiInputDevicePosition == 0 ? "none" :
                                                 (midiInputDevicePosition == 1 ? "auto" : sMIDIInputDevice.getSelectedItem().toString()));

            String logPath = etLogFile.getText().toString().trim();
            if (!logPath.equals(defaultLogPath) && !logPath.isEmpty()) {
                editor.putString("log_file", logPath);
            }
            else editor.remove("log_file");

            if (!wineDebugChannels.isEmpty()) {
                editor.putString("wine_debug_channels", String.join(",", wineDebugChannels));
            }
            else if (preferences.contains("wine_debug_channels")) editor.remove("wine_debug_channels");

            if (editor.commit()) {
                if (!restartApp) {
                    NavigationView navigationView = getActivity().findViewById(R.id.NavigationView);
                    navigationView.setCheckedItem(R.id.menu_item_containers);
                    FragmentManager fragmentManager = getParentFragmentManager();
                    fragmentManager.beginTransaction()
                        .replace(R.id.FLFragmentContainer, new ContainersFragment())
                        .commit();
                }
                else AppUtils.restartActivity(getActivity());
            }
        });

        return view;
    }

    private void loadBox64PresetSpinner(View view, final Spinner sBox64Preset) {
        final Context context = getContext();

        Runnable updateSpinner = () -> {
            Box64PresetManager.loadSpinner(sBox64Preset, preferences.getString("box64_preset", Box64Preset.DEFAULT));
        };

        updateSpinner.run();

        view.findViewById(R.id.BTAddBox64Preset).setOnClickListener((v) -> {
            Box64EditPresetDialog dialog = new Box64EditPresetDialog(context, null);
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        });
        view.findViewById(R.id.BTEditBox64Preset).setOnClickListener((v) -> {
            Box64EditPresetDialog dialog = new Box64EditPresetDialog(context, Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        });
        view.findViewById(R.id.BTDuplicateBox64Preset).setOnClickListener((v) -> {
            Box64PresetManager.duplicatePreset(context, Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            updateSpinner.run();
            sBox64Preset.setSelection(sBox64Preset.getCount()-1);
        });
        view.findViewById(R.id.BTRemoveBox64Preset).setOnClickListener((v) -> {
            final String presetId = Box64PresetManager.getSpinnerSelectedId(sBox64Preset);
            if (!presetId.startsWith(Box64Preset.CUSTOM)) {
                AppUtils.showToast(context, R.string.you_cannot_remove_this_preset);
                return;
            }
            ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_preset, () -> {
                Box64PresetManager.removePreset(context, presetId);
                updateSpinner.run();
            });
        });
    }

    private void removeInstalledWine(WineInfo wineInfo, Runnable onSuccess) {
        final Activity activity = getActivity();
        ContainerManager manager = new ContainerManager(activity);

        ArrayList<Container> containers = manager.getContainers();
        for (Container container : containers) {
            if (container.getWineVersion().equals(wineInfo.identifier())) {
                AppUtils.showToast(activity, R.string.unable_to_remove_this_wine_version);
                return;
            }
        }

        File installedWineDir = RootFS.find(activity).getInstalledWineDir();
        File wineDir = new File(wineInfo.path);
        File containerPatternFile = new File(installedWineDir, "container-pattern-"+wineInfo.fullVersion()+".tzst");

        if (!wineDir.isDirectory() || !containerPatternFile.isFile()) {
            AppUtils.showToast(activity, R.string.unable_to_remove_this_wine_version);
            return;
        }

        preloaderDialog.show(R.string.removing_wine);
        Executors.newSingleThreadExecutor().execute(() -> {
            FileUtils.delete(wineDir);
            FileUtils.delete(containerPatternFile);
            preloaderDialog.closeOnUiThread();
            if (onSuccess != null) activity.runOnUiThread(onSuccess);
        });
    }

    private void loadWineVersionSpinner(final View view, final Spinner sWineVersion) {
        Context context = getContext();
        final ArrayList<WineInfo> wineInfos = WineUtils.getInstalledWineInfos(context);
        sWineVersion.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, wineInfos));

        view.findViewById(R.id.BTInstallWine).setOnClickListener((v) -> selectWineFileForInstall());
        view.findViewById(R.id.BTRemoveWine).setOnClickListener((v) -> {
            WineInfo wineInfo = wineInfos.get(sWineVersion.getSelectedItemPosition());
            if (wineInfo != WineInfo.MAIN_WINE_INFO) {
                ContentDialog.confirm(getContext(), R.string.do_you_want_to_remove_this_wine_version, () -> {
                    removeInstalledWine(wineInfo, () -> loadWineVersionSpinner(view, sWineVersion));
                });
            }
        });
    }

    private void selectWineFileForInstall() {
        final Context context = getContext();
        selectWineFileCallback = (uri) -> {
            preloaderDialog.show(R.string.preparing_installation);
            WineUtils.extractWineFileForInstallAsync(context, uri, (wineDir) -> {
                if (wineDir != null) {
                    WineUtils.findWineVersionAsync(context, wineDir, (wineInfo) -> {
                        preloaderDialog.closeOnUiThread();
                        if (wineInfo == null) {
                            AppUtils.showToast(context, R.string.unable_to_install_wine);
                            return;
                        }

                        getActivity().runOnUiThread(() -> showWineInstallDialog(wineInfo));
                    });
                }
                else {
                    AppUtils.showToast(context, R.string.unable_to_install_wine);
                    preloaderDialog.closeOnUiThread();
                }
            });
        };

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        getActivity().startActivityFromFragment(this, intent, MainActivity.OPEN_FILE_REQUEST_CODE);
    }

    private void installWine(final WineInfo wineInfo) {
        Context context = getContext();
        File installedWineDir = RootFS.find(context).getInstalledWineDir();

        File wineDir = new File(installedWineDir, wineInfo.identifier());
        if (wineDir.isDirectory()) {
            AppUtils.showToast(context, R.string.unable_to_install_wine);
            return;
        }

        Intent intent = new Intent(context, XServerDisplayActivity.class);
        intent.putExtra("generate_wineprefix", true);
        intent.putExtra("wine_info", wineInfo);
        context.startActivity(intent);
    }

    private void showWineInstallDialog(final WineInfo wineInfo) {
        Context context = getContext();
        ContentDialog dialog = new ContentDialog(context, R.layout.wine_install_dialog);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setTitle(R.string.install_wine);
        dialog.setIcon(R.drawable.icon_wine);

        EditText etVersion = dialog.findViewById(R.id.ETVersion);
        etVersion.setText("Wine "+wineInfo.version+(wineInfo.subversion != null ? " ("+wineInfo.subversion+")" : ""));

        final EditText etSize = dialog.findViewById(R.id.ETSize);
        final AtomicLong totalSizeRef = new AtomicLong();
        FileUtils.getSizeAsync(new File(wineInfo.path), (size) -> {
            totalSizeRef.addAndGet(size);
            etSize.post(() -> etSize.setText(StringUtils.formatBytes(totalSizeRef.get())));
        });

        dialog.setOnConfirmCallback(() -> installWine(wineInfo));
        dialog.show();
    }

    private void loadWineDebugChannels(final View view, final ArrayList<String> debugChannels) {
        final Context context = getContext();
        LinearLayout container = view.findViewById(R.id.LLWineDebugChannels);
        container.removeAllViews();

        LayoutInflater inflater = LayoutInflater.from(context);
        View itemView = inflater.inflate(R.layout.wine_debug_channel_list_item, container, false);
        itemView.findViewById(R.id.TextView).setVisibility(View.GONE);
        itemView.findViewById(R.id.BTRemove).setVisibility(View.GONE);

        View addButton = itemView.findViewById(R.id.BTAdd);
        addButton.setVisibility(View.VISIBLE);
        addButton.setOnClickListener((v) -> {
            JSONArray jsonArray = null;
            try {
                jsonArray = new JSONArray(FileUtils.readString(context, "wine_debug_channels.json"));
            }
            catch (JSONException e) {}

            final String[] items = ArrayUtils.toStringArray(jsonArray);
            ContentDialog.showSelectionList(context, R.string.wine_debug_channel, items, true, (selectedPositions) -> {
                for (int selectedPosition : selectedPositions) if (!debugChannels.contains(items[selectedPosition])) debugChannels.add(items[selectedPosition]);
                loadWineDebugChannels(view, debugChannels);
            });
        });

        View resetButton = itemView.findViewById(R.id.BTReset);
        resetButton.setVisibility(View.VISIBLE);
        resetButton.setOnClickListener((v) -> {
            debugChannels.clear();
            debugChannels.addAll(Arrays.asList(DEFAULT_WINE_DEBUG_CHANNELS.split(",")));
            loadWineDebugChannels(view, debugChannels);
        });
        container.addView(itemView);

        for (int i = 0; i < debugChannels.size(); i++) {
            itemView = inflater.inflate(R.layout.wine_debug_channel_list_item, container, false);
            TextView textView = itemView.findViewById(R.id.TextView);
            textView.setText(debugChannels.get(i));
            final int index = i;
            itemView.findViewById(R.id.BTRemove).setOnClickListener((v) -> {
                debugChannels.remove(index);
                loadWineDebugChannels(view, debugChannels);
            });
            container.addView(itemView);
        }
    }

    public static void resetBox64Version(AppCompatActivity activity) {
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(activity);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString("box64_version", DefaultVersion.BOX64);
        editor.remove("current_box64_version");
        editor.apply();
    }

    private void loadMIDIInputDeviceSpinner(final Spinner sMIDIInputDevice, final String selectedValue) {
        Context context = getContext();
        MidiManager mm = (MidiManager)context.getSystemService(Context.MIDI_SERVICE);
        MidiDeviceInfo[] infos = mm.getDevices();

        if (!midiDeviceCallbackRegistered) {
            midiDeviceCallbackRegistered = true;
            mm.registerDeviceCallback(new MidiManager.DeviceCallback() {
                @Override
                public void onDeviceAdded(MidiDeviceInfo device) {
                    loadMIDIInputDeviceSpinner(sMIDIInputDevice, selectedValue);
                }

                @Override
                public void onDeviceRemoved(MidiDeviceInfo device) {
                    loadMIDIInputDeviceSpinner(sMIDIInputDevice, selectedValue);
                }
            }, new Handler(Looper.getMainLooper()));
        }

        ArrayList<String> items = new ArrayList<>();
        items.add(context.getString(R.string.none));
        items.add(context.getString(R.string.auto));

        for (MidiDeviceInfo info : infos) {
            if (info.getOutputPortCount() > 0) {
                Bundle properties = info.getProperties();
                items.add(properties.getString(MidiDeviceInfo.PROPERTY_NAME));
            }
        }

        sMIDIInputDevice.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, items));

        if (selectedValue.equals("none")) {
            sMIDIInputDevice.setSelection(0, false);
        }
        else if (selectedValue.equals("auto") || !AppUtils.setSpinnerSelectionFromValue(sMIDIInputDevice, selectedValue)) {
            sMIDIInputDevice.setSelection(1, false);
        }
    }

    private void loadGamepadPlayerConfigs(View view) {
        LinearLayout container = view.findViewById(R.id.LLGamepadPlayer);
        view.findViewById(R.id.BTResetGamepadPlayerConfigs).setOnClickListener((v) -> {
            ContentDialog.confirm(v.getContext(), R.string.do_you_want_to_reset_configurations, () -> {
                for (int i = 0; i < container.getChildCount(); i++) container.getChildAt(i).setTag("");
            });
        });

        for (int i = 0; i < container.getChildCount(); i++) {
            final View child = container.getChildAt(i);
            child.setTag(preferences.getString("gamepad_player"+i, ""));
            final byte slot = (byte)i;
            child.setOnClickListener((v) -> (new GamepadPlayerConfigDialog(child, slot)).show());
        }
    }

    private void putGamepadPlayerConfigs(View view, SharedPreferences.Editor editor) {
        LinearLayout container = view.findViewById(R.id.LLGamepadPlayer);
        for (int i = 0; i < container.getChildCount(); i++) {
            View child = container.getChildAt(i);
            String config = child.getTag().toString();
            String key = "gamepad_player"+i;
            if (!config.isEmpty()) {
                editor.putString(key, child.getTag().toString());
            }
            else editor.remove(key);
        }
    }
}
