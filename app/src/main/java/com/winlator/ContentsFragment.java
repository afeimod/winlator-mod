package com.winlator;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.ProgressBar;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.content.res.AppCompatResources;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.winlator.container.Container;
import com.winlator.container.ContainerManager;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.contentdialog.ContentInfoDialog;
import com.winlator.contentdialog.ContentUntrustedDialog;
import com.winlator.contents.ContentProfile;
import com.winlator.contents.ContentsManager;
import com.winlator.contents.Downloader;
import com.winlator.core.AppUtils;
import com.winlator.core.DownloadProgressDialog;
import com.winlator.core.FileUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

public class ContentsFragment extends Fragment {
    private RecyclerView recyclerView;
    private View emptyText;
    private ContentsManager manager;
    private ContentProfile.ContentType currentContentType = ContentProfile.ContentType.CONTENT_TYPE_WINE;
    private Spinner sContentType;

    private void safeRunOnUiThread(Runnable action) {
        Activity activity = getActivity();
        if (activity != null && isAdded() && !activity.isFinishing() && !activity.isDestroyed()) {
            activity.runOnUiThread(action);
        }
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        manager = new ContentsManager(requireContext());
        manager.syncContents();
    }

    @Override
    public void onDestroy() {
        Context context = getContext();
        if (context != null) FileUtils.clear(context.getCacheDir());
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        Executors.newSingleThreadExecutor().execute(() -> {
            String json = Downloader.downloadString(ContentsManager.REMOTE_PROFILES_URL);
            if (json != null) {
                safeRunOnUiThread(() -> {
                    manager.setRemoteProfiles(json);
                    loadContentList();
                });
            }
        });
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Activity activity = getActivity();
        if (activity instanceof AppCompatActivity) {
            AppCompatActivity appCompatActivity = (AppCompatActivity) activity;
            if (appCompatActivity.getSupportActionBar() != null) {
                appCompatActivity.getSupportActionBar().setTitle(R.string.contents);
            }
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        ViewGroup layout = (ViewGroup) inflater.inflate(R.layout.contents_fragment, container, false);

        sContentType = layout.findViewById(R.id.SContentType);
        updateContentTypeSpinner(sContentType);
        sContentType.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                currentContentType = ContentProfile.ContentType.values()[position];
                loadContentList();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {}
        });

        emptyText = layout.findViewById(R.id.TVEmptyText);

        View btInstallContent = layout.findViewById(R.id.BTInstallContent);
        btInstallContent.setOnClickListener(v -> ContentDialog.confirm(getContext(), getString(R.string.do_you_want_to_install_content) + " " + getString(R.string.pls_make_sure_content_trustworthy) + " "
                + getString(R.string.content_suffix_is_wcp_packed_xz_zst) + '\n' + getString(R.string.get_more_contents_form_github), () -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            startActivityForResult(intent, MainActivity.OPEN_FILE_REQUEST_CODE);
        }));

        recyclerView = layout.findViewById(R.id.RecyclerView);
        recyclerView.setLayoutManager(new LinearLayoutManager(recyclerView.getContext()));
        recyclerView.addItemDecoration(new DividerItemDecoration(recyclerView.getContext(), DividerItemDecoration.VERTICAL));
        loadContentList();

        return layout;
    }

    private void updateContentTypeSpinner(Spinner spinner) {
        List<String> typeList = new ArrayList<>();
        for (ContentProfile.ContentType type : ContentProfile.ContentType.values())
            typeList.add(type.toString());
        
        Context context = getContext();
        if (context != null) {
            spinner.setAdapter(new ArrayAdapter<>(context, android.R.layout.simple_spinner_dropdown_item, typeList));
            spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                    currentContentType = ContentProfile.ContentType.values()[position];
                    updateContentsListView();
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {}
            });
        }
    }

    private void updateContentsListView() {
        List<ContentProfile> profiles = manager.getProfiles(currentContentType);
        if (profiles.isEmpty()) {
            recyclerView.setVisibility(View.GONE);
            emptyText.setVisibility(View.VISIBLE);
        } else {
            recyclerView.setVisibility(View.VISIBLE);
            emptyText.setVisibility(View.GONE);
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        if (requestCode == MainActivity.OPEN_FILE_REQUEST_CODE && resultCode == Activity.RESULT_OK && data != null) {
            final Activity activity = getActivity();
            if (activity == null) return;

            DownloadProgressDialog progressDialog = new DownloadProgressDialog(activity);
            progressDialog.show(R.string.installing_content);
            try {
                ContentsManager.OnInstallFinishedCallback callback = new ContentsManager.OnInstallFinishedCallback() {
                    private boolean isExtracting = true;

                    @Override
                    public void onFailed(ContentsManager.InstallFailedReason reason, Exception e) {
                        int msgId = switch (reason) {
                            case ERROR_BADTAR -> R.string.file_cannot_be_recognied;
                            case ERROR_NOPROFILE -> R.string.profile_not_found_in_content;
                            case ERROR_BADPROFILE -> R.string.profile_cannot_be_recognized;
                            case ERROR_EXIST -> R.string.content_already_exist;
                            case ERROR_MISSINGFILES -> R.string.content_is_incomplete;
                            case ERROR_UNTRUSTPROFILE -> R.string.content_cannot_be_trusted;
                            default -> R.string.unable_to_install_content;
                        };
                        safeRunOnUiThread(() -> ContentDialog.alert(getContext(), activity.getString(R.string.install_failed) + ": " + activity.getString(msgId), progressDialog::closeOnUiThread));
                    }

                    @Override
                    public void onSucceed(ContentProfile profile) {
                        if (isExtracting) {
                            final ContentsManager.OnInstallFinishedCallback callback1 = this;
                            safeRunOnUiThread(() -> {
                                ContentInfoDialog dialog = new ContentInfoDialog(getContext(), profile);
                                TextView confirmBtn = dialog.findViewById(R.id.BTConfirm);
                                if (confirmBtn != null) confirmBtn.setText(R.string._continue);
                                dialog.setOnConfirmCallback(() -> {
                                    isExtracting = false;
                                    List<ContentProfile.ContentFile> untrustedFiles = manager.getUnTrustedContentFiles(profile);
                                    if (!untrustedFiles.isEmpty()) {
                                        ContentUntrustedDialog untrustedDialog = new ContentUntrustedDialog(getContext(), untrustedFiles);
                                        untrustedDialog.setOnCancelCallback(progressDialog::closeOnUiThread);
                                        untrustedDialog.setOnConfirmCallback(() -> manager.finishInstallContent(profile, callback1));
                                        untrustedDialog.show();
                                    } else manager.finishInstallContent(profile, callback1);
                                });
                                dialog.setOnCancelCallback(progressDialog::closeOnUiThread);
                                dialog.show();
                            });

                        } else {
                            progressDialog.closeOnUiThread();
                            safeRunOnUiThread(() -> {
                                ContentDialog.alert(getContext(), R.string.content_installed_success, null);
                                manager.syncContents();
                                boolean flashAfter = currentContentType == profile.type;
                                currentContentType = profile.type;
                                AppUtils.setSpinnerSelectionFromValue(sContentType, currentContentType.toString());
                                if (flashAfter) loadContentList();
                            });
                        }
                    }
                };
                Executors.newSingleThreadExecutor().execute(() -> manager.extraContentFile(data.getData(), callback, progressDialog::setProgress));
            } catch (Exception e) {
                progressDialog.closeOnUiThread();
                AppUtils.showToast(getContext(), R.string.unable_to_import_profile);
            }
        }
    }

    private void loadContentList() {
        if (!isAdded()) return;
        List<ContentProfile> profiles = manager.getProfiles(currentContentType);

        if (profiles.isEmpty()) {
            emptyText.setVisibility(View.VISIBLE);
            recyclerView.setVisibility(View.GONE);
        } else {
            emptyText.setVisibility(View.GONE);
            recyclerView.setVisibility(View.VISIBLE);
            recyclerView.setAdapter(new ContentItemAdapter(profiles));
        }
    }

    private class ContentItemAdapter extends RecyclerView.Adapter<ContentItemAdapter.ViewHolder> {
        private final List<ContentProfile> data;

        private static class ViewHolder extends RecyclerView.ViewHolder {
            private final ImageView ivIcon;
            private final TextView tvVersionName;
            private final TextView tvVersionCode;
            private final ImageButton ibMenu;
            private final ImageButton ibDownload;
            private final ProgressBar progressBar;

            public ViewHolder(@NonNull View view) {
                super(view);
                ivIcon = view.findViewById(R.id.IVIcon);
                tvVersionName = view.findViewById(R.id.TVVersionName);
                tvVersionCode = view.findViewById(R.id.TVVersionCode);
                ibMenu = view.findViewById(R.id.BTMenu);
                ibDownload = view.findViewById(R.id.BTDownload);
                progressBar = view.findViewById(R.id.Progress);
            }
        }

        public ContentItemAdapter(List<ContentProfile> data) {
            this.data = data;
        }

        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            return new ContentItemAdapter.ViewHolder(LayoutInflater.from(parent.getContext()).inflate(R.layout.content_list_item, parent, false));
        }

        @Override
        public void onViewRecycled(@NonNull ViewHolder holder) {
            holder.ibMenu.setOnClickListener(null);
            super.onViewRecycled(holder);
        }

        @SuppressLint({"StringFormatInvalid", "SetTextI18n"})
        @Override
        public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
            final ContentProfile profile = data.get(position);
            Context context = getContext();
            if (context == null) return;

            int iconId = switch (profile.type) {
                case CONTENT_TYPE_WINE -> R.drawable.icon_wine;
                default -> R.drawable.icon_settings;
            };
            holder.ivIcon.setBackground(AppCompatResources.getDrawable(context, iconId));

            holder.tvVersionName.setText(context.getString(R.string.version) + ": " + profile.verName);
            holder.tvVersionCode.setText(context.getString(R.string.version_code) + ": " + profile.verCode);
            holder.ibMenu.setVisibility(profile.remoteUrl == null ? View.VISIBLE : View.GONE);
            holder.ibMenu.setOnClickListener(v -> {
                PopupMenu selectionMenu = new PopupMenu(context, holder.ibMenu);
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q)
                    selectionMenu.setForceShowIcon(true);
                selectionMenu.inflate(R.menu.content_popup_menu);
                selectionMenu.setOnMenuItemClickListener(item -> {
                    int itemId = item.getItemId();
                    if (itemId == R.id.content_info) {
                        new ContentInfoDialog(context, profile).show();
                    } else if (itemId == R.id.remove_content) {
                        ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_content, () -> {
                            if (profile.type == ContentProfile.ContentType.CONTENT_TYPE_WINE) {
                                ContainerManager containerManager = new ContainerManager(context);
                                for (Container container : containerManager.getContainers()) {
                                    if (container.getWineVersion().equals(ContentsManager.getEntryName(profile))) {
                                        ContentDialog.alert(context, String.format(getString(R.string.unable_to_remove_content_since_container_using), container.getName()), null);
                                        return;
                                    }
                                }
                            }
                            manager.removeContent(profile);
                            loadContentList();
                        });
                    }
                    return true;
                });
                selectionMenu.show();
            });
            holder.ibDownload.setVisibility((profile.remoteUrl != null) && (holder.progressBar.getVisibility() == View.GONE) ? View.VISIBLE : View.GONE);
            holder.ibDownload.setOnClickListener(v -> {
                final Activity activity = getActivity();
                if (activity == null) return;

                DownloadProgressDialog progressDialog = new DownloadProgressDialog(activity);
                AtomicBoolean isCancelled = new AtomicBoolean(false);
                progressDialog.show(R.string.downloading_file, () -> {
                    isCancelled.set(true);
                    progressDialog.close();
                });

                Executors.newSingleThreadExecutor().execute(() -> {
                    long timestamp = System.currentTimeMillis();
                    File output = new File(context.getCacheDir(), "temp_" + timestamp);
                    
                    if (Downloader.downloadFile(profile.remoteUrl, output, progressDialog::setProgress, isCancelled)) {
                        safeRunOnUiThread(() -> {
                            progressDialog.close();
                            final Intent intent = new Intent();
                            intent.setData(Uri.parse(output.getAbsolutePath()));
                            onActivityResult(MainActivity.OPEN_FILE_REQUEST_CODE, Activity.RESULT_OK, intent);
                        });
                    } else {
                        if (!isCancelled.get()) {
                            safeRunOnUiThread(() -> {
                                progressDialog.close();
                                ContentDialog.alert(context, R.string.unable_to_download_file, null);
                            });
                        }
                    }
                });
            });
        }

        @Override
        public int getItemCount() {
            return data.size();
        }
    }
}
