package com.winlator;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
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
import com.winlator.contents.ContentProfile;
import com.winlator.contents.ContentsManager;
import com.winlator.core.AppUtils;
import com.winlator.core.ArrayUtils;
import com.winlator.core.Callback;
import com.winlator.core.DefaultVersion;
import com.winlator.core.FileUtils;
import com.winlator.core.PreloaderDialog;
import com.winlator.core.StringUtils;
import com.winlator.core.WineInfo;
import com.winlator.core.WineUtils;
import com.winlator.fex.FEXEditPresetDialog;
import com.winlator.fex.FEXPreset;
import com.winlator.fex.FEXPresetManager;
import com.winlator.inputcontrols.ExternalController;
import com.winlator.midi.MidiManager;
import com.winlator.xenvironment.ImageFs;
import com.winlator.xenvironment.ImageFsInstaller;

import org.json.JSONArray;
import org.json.JSONException;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Executors;

public class SettingsFragment extends Fragment {
    public static final String DEFAULT_WINE_DEBUG_CHANNELS = "warn,err,fixme";
    private Callback<Uri> selectWineFileCallback;
    private Callback<Uri> installSoundFontCallback;
    private PreloaderDialog preloaderDialog;
    private SharedPreferences preferences;

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
            if (selectWineFileCallback != null) {
                try {
                    if (selectWineFileCallback != null && data != null) selectWineFileCallback.call(data.getData());
                }
                catch (Exception e) {
                    AppUtils.showToast(getContext(), R.string.unable_to_import_profile);
                }
                selectWineFileCallback = null;
            } else if (installSoundFontCallback != null) {
                installSoundFontCallback.call(data.getData());
                installSoundFontCallback = null;
            }
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.settings_fragment, container, false);
        final Context context = getContext();
        preferences = PreferenceManager.getDefaultSharedPreferences(context);

        final Spinner sBox64Preset = view.findViewById(R.id.SBox64Preset);
        loadBox64PresetSpinner(view, sBox64Preset);

        final Spinner sFEXPreset = view.findViewById(R.id.SFEXPreset);
        loadFEXPresetSpinner(view, sFEXPreset);

        final Spinner sMIDISoundFont = view.findViewById(R.id.SMIDISoundFont);
        final View btInstallSF = view.findViewById(R.id.BTInstallSF);
        final View btRemoveSF = view.findViewById(R.id.BTRemoveSF);

        MidiManager.loadSFSpinnerWithoutDisabled(sMIDISoundFont);
        btInstallSF.setOnClickListener(v -> {
            installSoundFontCallback = uri -> {
                PreloaderDialog dialog = new PreloaderDialog(requireActivity());
                dialog.showOnUiThread(R.string.installing_content);
                MidiManager.installSF2File(context, uri, new MidiManager.OnSoundFontInstalledCallback() {
                    @Override
                    public void onSuccess() {
                        dialog.closeOnUiThread();
                        requireActivity().runOnUiThread(() -> {
                            ContentDialog.alert(context, R.string.sound_font_installed_success, null);
                            MidiManager.loadSFSpinnerWithoutDisabled(sMIDISoundFont);
                        });
                    }

                    @Override
                    public void onFailed(int reason) {
                        dialog.closeOnUiThread();
                        int resId = switch (reason) {
                            case MidiManager.ERROR_BADFORMAT -> R.string.sound_font_bad_format;
                            case MidiManager.ERROR_EXIST -> R.string.sound_font_already_exist;
                            default -> R.string.sound_font_installed_failed;
                        };
                        requireActivity().runOnUiThread(() -> ContentDialog.alert(context, resId, null));
                    }
                });
            };
            openFile();
        });
        btRemoveSF.setOnClickListener(v -> {
            if (sMIDISoundFont.getSelectedItemPosition() != 0) {
                ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_sound_font, () -> {
                    if (MidiManager.removeSF2File(context, sMIDISoundFont.getSelectedItem().toString())) {
                        AppUtils.showToast(context, R.string.sound_font_removed_success);
                        MidiManager.loadSFSpinnerWithoutDisabled(sMIDISoundFont);
                    } else
                        AppUtils.showToast(context, R.string.sound_font_removed_failed);
                });
            } else
                AppUtils.showToast(context, R.string.cannot_remove_default_sound_font);
        });

        final CheckBox cbHaptics = view.findViewById(R.id.CBHaptics);
        cbHaptics.setChecked(preferences.getBoolean("haptics", true));

        final CheckBox cbUseDRI3 = view.findViewById(R.id.CBUseDRI3);
        cbUseDRI3.setChecked(preferences.getBoolean("use_dri3", true));

        final CheckBox cbUseXR = view.findViewById(R.id.CBUseXR);
        cbUseXR.setChecked(preferences.getBoolean("use_xr", true));
        if (!XrActivity.isSupported()) {
            cbUseXR.setVisibility(View.GONE);
        }

        final CheckBox cbUseTX11 = view.findViewById(R.id.CBUseTX11);
        cbUseTX11.setChecked(preferences.getBoolean("use_tx11", false));

        final CheckBox cbEnableWineDebug = view.findViewById(R.id.CBEnableWineDebug);
        cbEnableWineDebug.setChecked(preferences.getBoolean("enable_wine_debug", false));

        final ArrayList<String> wineDebugChannels = new ArrayList<>(Arrays.asList(preferences.getString("wine_debug_channels", DEFAULT_WINE_DEBUG_CHANNELS).split(",")));
        loadWineDebugChannels(view, wineDebugChannels);

        final CheckBox cbEnableBox64Logs = view.findViewById(R.id.CBEnableBox64Logs);
        cbEnableBox64Logs.setChecked(preferences.getBoolean("enable_box64_logs", false));

        final CheckBox cbEnableStartupDesktopLogs = view.findViewById(R.id.CBEnableStartupDesktopLogs);
        cbEnableStartupDesktopLogs.setChecked(preferences.getBoolean("enable_startup_desktop_logs", false));

        final TextView tvCursorSpeed = view.findViewById(R.id.TVCursorSpeed);
        final SeekBar sbCursorSpeed = view.findViewById(R.id.SBCursorSpeed);
        sbCursorSpeed.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                tvCursorSpeed.setText(progress+"%");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });
        sbCursorSpeed.setProgress((int)(preferences.getFloat("cursor_speed", 1.0f) * 100));

        final RadioGroup rgTriggerType = view.findViewById(R.id.RGTriggerType);
        final View btHelpTriggerMode = view.findViewById(R.id.BTHelpTriggerMode);
        List<Integer> triggerRbIds = List.of(R.id.RBTriggerIsButton, R.id.RBTriggerIsAxis, R.id.RBTriggerIsMixed);
        int triggerType = preferences.getInt("trigger_type", ExternalController.TRIGGER_IS_AXIS);

        if (triggerType >= 0 && triggerType < triggerRbIds.size()) {
            ((RadioButton) (rgTriggerType.findViewById(triggerRbIds.get(triggerType)))).setChecked(true);
        }
        btHelpTriggerMode.setOnClickListener(v -> AppUtils.showHelpBox(context, v, R.string.help_trigger_mode));

        final CheckBox cbEnableFileProvider = view.findViewById(R.id.CBEnableFileProvider);
        final View btHelpFileProvider = view.findViewById(R.id.BTHelpFileProvider);

        cbEnableFileProvider.setChecked(preferences.getBoolean("enable_file_provider", true));
        cbEnableFileProvider.setOnClickListener(v -> AppUtils.showToast(context, R.string.take_effect_next_startup));
        btHelpFileProvider.setOnClickListener(v -> AppUtils.showHelpBox(context, v, R.string.help_file_provider));

        loadInstalledWineList(view);

        view.findViewById(R.id.BTSelectWineFile).setOnClickListener((v) -> {
            ContentDialog.alert(context, R.string.msg_warning_install_wine, this::selectWineFileForInstall);
        });

        view.findViewById(R.id.BTReInstallImagefs).setOnClickListener(v -> {
            ContentDialog.confirm(context, R.string.do_you_want_to_reinstall_imagefs, () -> ImageFsInstaller.installFromAssets((MainActivity) getActivity()));
        });

        view.findViewById(R.id.BTConfirm).setOnClickListener((v) -> {
            SharedPreferences.Editor editor = preferences.edit();
            editor.putString("box64_preset", Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            editor.putString("fex_preset", FEXPresetManager.getSpinnerSelectedId(sFEXPreset));
            editor.putBoolean("haptics", cbHaptics.isChecked());
            editor.putBoolean("use_dri3", cbUseDRI3.isChecked());
            editor.putBoolean("use_xr", cbUseXR.isChecked());
            editor.putBoolean("use_tx11", cbUseTX11.isChecked());
            editor.putFloat("cursor_speed", sbCursorSpeed.getProgress() / 100.0f);
            editor.putBoolean("enable_wine_debug", cbEnableWineDebug.isChecked());
            editor.putBoolean("enable_box64_logs", cbEnableBox64Logs.isChecked());
            editor.putBoolean("enable_startup_desktop_logs", cbEnableStartupDesktopLogs.isChecked());
            editor.putInt("trigger_type", triggerRbIds.indexOf(rgTriggerType.getCheckedRadioButtonId()));
            editor.putBoolean("enable_file_provider", cbEnableFileProvider.isChecked());

            if (!wineDebugChannels.isEmpty()) {
                editor.putString("wine_debug_channels", String.join(",", wineDebugChannels));
            }
            else if (preferences.contains("wine_debug_channels")) editor.remove("wine_debug_channels");

            if (editor.commit()) {
                NavigationView navigationView = getActivity().findViewById(R.id.NavigationView);
                navigationView.setCheckedItem(R.id.main_menu_containers);
                FragmentManager fragmentManager = getParentFragmentManager();
                fragmentManager.beginTransaction()
                    .replace(R.id.FLFragmentContainer, new ContainersFragment())
                    .commit();
            }
        });

        return view;
    }

    private void loadBox64PresetSpinner(View view, final Spinner sBox64Preset) {
        final Context context = getContext();

        Runnable updateSpinner = () -> {
            Box64PresetManager.loadSpinner(sBox64Preset, preferences.getString("box64_preset", Box64Preset.COMPATIBILITY));
        };

        View.OnClickListener onAddPreset = (v) -> {
            Box64EditPresetDialog dialog = new Box64EditPresetDialog(context, null);
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        };

        View.OnClickListener onEditPreset = (v) -> {
            Box64EditPresetDialog dialog = new Box64EditPresetDialog(context, Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        };

        View.OnClickListener onDuplicatePreset = (v) -> ContentDialog.confirm(context, R.string.do_you_want_to_duplicate_this_preset, () -> {
            Box64PresetManager.duplicatePreset(context, Box64PresetManager.getSpinnerSelectedId(sBox64Preset));
            updateSpinner.run();
            sBox64Preset.setSelection(sBox64Preset.getCount()-1);
        });

        View.OnClickListener onRemovePreset = (v) -> {
            final String presetId = Box64PresetManager.getSpinnerSelectedId(sBox64Preset);
            if (!presetId.startsWith(Box64Preset.CUSTOM)) {
                AppUtils.showToast(context, R.string.you_cannot_remove_this_preset);
                return;
            }
            ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_preset, () -> {
                Box64PresetManager.removePreset(context, presetId);
                updateSpinner.run();
            });
        };

        updateSpinner.run();

        view.findViewById(R.id.BTAddBox64Preset).setOnClickListener(onAddPreset);
        view.findViewById(R.id.BTEditBox64Preset).setOnClickListener(onEditPreset);
        view.findViewById(R.id.BTDuplicateBox64Preset).setOnClickListener(onDuplicatePreset);
        view.findViewById(R.id.BTRemoveBox64Preset).setOnClickListener(onRemovePreset);
    }

    private void loadFEXPresetSpinner(View view, final Spinner sFEXPreset) {
        final Context context = getContext();

        Runnable updateSpinner = () -> {
            FEXPresetManager.loadSpinner(sFEXPreset, preferences.getString("fex_preset", FEXPreset.COMPATIBILITY));
        };

        View.OnClickListener onAddPreset = (v) -> {
            FEXEditPresetDialog dialog = new FEXEditPresetDialog(context, null);
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        };

        View.OnClickListener onEditPreset = (v) -> {
            FEXEditPresetDialog dialog = new FEXEditPresetDialog(context, FEXPresetManager.getSpinnerSelectedId(sFEXPreset));
            dialog.setOnConfirmCallback(updateSpinner);
            dialog.show();
        };

        View.OnClickListener onDuplicatePreset = (v) -> ContentDialog.confirm(context, R.string.do_you_want_to_duplicate_this_preset, () -> {
            FEXPresetManager.duplicatePreset(context, FEXPresetManager.getSpinnerSelectedId(sFEXPreset));
            updateSpinner.run();
            sFEXPreset.setSelection(sFEXPreset.getCount()-1);
        });

        View.OnClickListener onRemovePreset = (v) -> {
            final String presetId = FEXPresetManager.getSpinnerSelectedId(sFEXPreset);
            if (!presetId.startsWith(FEXPreset.CUSTOM)) {
                AppUtils.showToast(context, R.string.you_cannot_remove_this_preset);
                return;
            }
            ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_preset, () -> {
                FEXPresetManager.removePreset(context, presetId);
                updateSpinner.run();
            });
        };

        updateSpinner.run();

        view.findViewById(R.id.BTAddFEXPreset).setOnClickListener(onAddPreset);
        view.findViewById(R.id.BTEditFEXPreset).setOnClickListener(onEditPreset);
        view.findViewById(R.id.BTDuplicateFEXPreset).setOnClickListener(onDuplicatePreset);
        view.findViewById(R.id.BTRemoveFEXPreset).setOnClickListener(onRemovePreset);
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

        String suffix = wineInfo.fullVersion()+"-"+wineInfo.getArch();
        File installedWineDir = ImageFs.find(activity).getInstalledWineDir();
        File wineDir = new File(wineInfo.path);
        File containerPatternFile = new File(installedWineDir, "container-pattern-"+suffix+".tzst");

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

    private void loadInstalledWineList(final View view) {
        Context context = getContext();
        LinearLayout container = view.findViewById(R.id.LLInstalledWineList);
        container.removeAllViews();
        ArrayList<WineInfo> wineInfos = WineUtils.getInstalledWineInfos(context);

        LayoutInflater inflater = LayoutInflater.from(context);
        for (final WineInfo wineInfo : wineInfos) {
            View itemView = inflater.inflate(R.layout.installed_wine_list_item, container, false);
            ((TextView)itemView.findViewById(R.id.TVTitle)).setText(wineInfo.toString());
            if (wineInfo != WineInfo.MAIN_WINE_VERSION) {
                View removeButton = itemView.findViewById(R.id.BTRemove);
                removeButton.setVisibility(View.VISIBLE);
                removeButton.setOnClickListener((v) -> {
                    ContentDialog.confirm(getContext(), R.string.do_you_want_to_remove_this_wine_version, () -> {
                        removeInstalledWine(wineInfo, () -> loadInstalledWineList(view));
                    });
                });
            }
            container.addView(itemView);
        }
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

                        getActivity().runOnUiThread(() -> showWineInstallOptionsDialog(wineInfo));
                    });
                }
                else {
                    AppUtils.showToast(context, R.string.unable_to_install_wine);
                    preloaderDialog.closeOnUiThread();
                }
            });
        };
        openFile();
    }

    private void openFile() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        getActivity().startActivityFromFragment(this, intent, MainActivity.OPEN_FILE_REQUEST_CODE);
    }

    private void installWine(final WineInfo wineInfo) {
        Context context = getContext();
        File installedWineDir = ImageFs.find(context).getInstalledWineDir();

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

    private void showWineInstallOptionsDialog(final WineInfo wineInfo) {
        Context context = getContext();
        ContentDialog dialog = new ContentDialog(context, R.layout.wine_install_options_dialog);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setTitle(R.string.install_wine);
        dialog.setIcon(R.drawable.icon_wine);

        EditText etVersion = dialog.findViewById(R.id.ETVersion);
        etVersion.setText("Wine "+wineInfo.version+(wineInfo.subversion != null ? " ("+wineInfo.subversion+")" : ""));

        Spinner sArch = dialog.findViewById(R.id.SArch);
        List<String> archList = wineInfo.isWin64() ? Arrays.asList("x86", "x86_64") : Arrays.asList("x86");
        sArch.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, archList));
        sArch.setSelection(archList.size()-1);

        dialog.setOnConfirmCallback(() -> {
            wineInfo.setArch(sArch.getSelectedItem().toString());
            installWine(wineInfo);
        });
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
            ContentDialog.showMultipleChoiceList(context, R.string.wine_debug_channel, items, (selectedPositions) -> {
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

    public static void loadBox64VersionSpinner(Context context, ContentsManager manager, Spinner spinner) {
        String[] originalItems = context.getResources().getStringArray(R.array.box64_version_entries);
        List<String> itemList = new ArrayList<>(Arrays.asList(originalItems));
        for (ContentProfile profile : manager.getProfiles(ContentProfile.ContentType.CONTENT_TYPE_BOX64)) {
            String entryName = ContentsManager.getEntryName(profile);
            int firstDashIndex = entryName.indexOf('-');
            itemList.add(entryName.substring(firstDashIndex + 1));
        }
        spinner.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, itemList));
    }
}
