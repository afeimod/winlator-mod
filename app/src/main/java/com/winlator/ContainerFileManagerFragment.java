package com.winlator;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.RecyclerView;

import com.winlator.container.Container;
import com.winlator.container.FileInfo;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.contentdialog.FileInfoDialog;
import com.winlator.core.AppUtils;
import com.winlator.core.FileUtils;
import com.winlator.core.StringUtils;
import com.winlator.core.WineUtils;
import com.winlator.win32.MSIcon;
import com.winlator.win32.MSLink;
import com.winlator.win32.PEParser;
import com.winlator.xenvironment.RootFS;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class ContainerFileManagerFragment extends BaseFileManagerFragment<FileInfo> {
    private final int containerId;
    private String startPath;
    private Container container;

    public ContainerFileManagerFragment(int containerId) {
        this(containerId, null);
    }

    public ContainerFileManagerFragment(int containerId, String startPath) {
        this.containerId = containerId;
        this.startPath = startPath;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        container = manager.getContainerById(containerId);
        viewStyle = ViewStyle.valueOf(preferences.getString("container_file_manager_view_style", "GRID"));
        
        if (startPath != null) {
            setCurrentWorkingPath(WineUtils.unixToDOSPath(startPath, container));
            startPath = null;
        }
    }

    @Override
    public void refreshContent() {
        super.refreshContent();

        FileInfo parent = !folderStack.isEmpty() ? folderStack.peek() : null;
        ArrayList<FileInfo> files = manager.loadFiles(container, parent);
        recyclerView.setAdapter(new FileInfoAdapter(files));
        emptyTextView.setVisibility(files.isEmpty() ? View.VISIBLE : View.GONE);
        updateActionBarTitle();
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater menuInflater) {
        menuInflater.inflate(R.menu.container_file_manager_menu, menu);
        refreshViewStyleMenuItem(menu.findItem(R.id.menu_item_view_style));
    }

    private void createFolder() {
        clearClipboard();
        if (folderStack.isEmpty()) return;

        ContentDialog.prompt(getContext(), R.string.new_folder, null, (name) -> {
            File file = new File(folderStack.peek().toFile(), name);
            if (file.isDirectory()) {
                AppUtils.showToast(getContext(), R.string.there_already_file_with_that_name);
            }
            else {
                file.mkdir();
                refreshContent();
            }
        });
    }

    private void instantiateClipboard(FileInfo file, boolean cutMode) {
        clearClipboard();
        clipboard = new Clipboard(new File[]{new File(file.path)}, cutMode);
        pasteButton.setVisibility(View.VISIBLE);
    }

    private void addFavorite(FileInfo file) {
        Context context = getContext();
        File favoritesDir = new File(container.getUserDir(), context.getString(R.string.favorites));
        File targetFile = new File(favoritesDir, FileUtils.getBasename(file.name)+".lnk");

        if (!targetFile.exists()) {
            MSLink.LinkInfo linkInfo = new MSLink.LinkInfo();
            linkInfo.targetPath = WineUtils.unixToDOSPath(file.path, container);
            linkInfo.isDirectory = file.type == FileInfo.Type.DIRECTORY;
            boolean success = MSLink.createFile(linkInfo, targetFile);
            if (success) AppUtils.showToast(context, R.string.file_added_to_favorites);
        }
    }

    @Override
    protected void pasteFiles() {
        if (folderStack.isEmpty()) {
            clearClipboard();
            AppUtils.showToast(getContext(), R.string.you_cannot_paste_files_here);
            return;
        }

        clipboard.targetDir = folderStack.peek().toFile();
        super.pasteFiles();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        int itemId = menuItem.getItemId();
        switch (itemId) {
            case R.id.menu_item_home:
                folderStack.clear();
                refreshContent();
                return true;
            case R.id.menu_item_view_style:
                setViewStyle(viewStyle == ViewStyle.GRID ? ViewStyle.LIST : ViewStyle.GRID);
                preferences.edit().putString("container_file_manager_view_style", viewStyle.name()).apply();
                refreshViewStyleMenuItem(menuItem);
                return true;
            case R.id.menu_item_new_folder:
                createFolder();
                return true;
            default:
                return super.onOptionsItemSelected(menuItem);
        }
    }

    private void setCurrentWorkingPath(String dosPath) {
        dosPath = StringUtils.removeEndSlash(dosPath);
        String[] names = dosPath.split("\\\\");
        String basePath = "";

        folderStack.clear();
        for (String name : names) {
            if (!name.isEmpty()) {
                dosPath = WineUtils.dosToUnixPath(basePath+name, container);
                if (basePath.isEmpty() && name.matches("[A-Za-z]:")) {
                    folderStack.push(new FileInfo(container, name, dosPath, FileInfo.Type.DRIVE));
                }
                else folderStack.push(new FileInfo(container, dosPath, FileInfo.Type.DIRECTORY));
                basePath += name+"\\";
            }
        }

        updateActionBarTitle();
    }

    private String getCurrentWorkingPath() {
        if (!folderStack.isEmpty()) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < folderStack.size(); i++) {
                if (i > 0) sb.append("\\");
                sb.append(folderStack.elementAt(i).getDisplayName());
            }

            if (folderStack.size() == 1) sb.append("\\");
            return sb.toString();
        }
        else return "";
    }

    private void updateActionBarTitle() {
        AppCompatActivity activity = (AppCompatActivity)getActivity();
        ActionBar actionBar = activity.getSupportActionBar();

        if (!folderStack.isEmpty()) {
            actionBar.setHomeAsUpIndicator(R.drawable.icon_action_bar_back);
            actionBar.setTitle(getCurrentWorkingPath());
        }
        else {
            actionBar.setHomeAsUpIndicator(R.drawable.icon_action_bar_menu);
            actionBar.setTitle(getHomeTitle());
        }
    }

    @Override
    protected String getHomeTitle() {
        return container.getName();
    }

    private static class LoadIconTask {
        private final Executor executor = Executors.newSingleThreadExecutor();
        private final ContainerFileManagerFragment fragment;
        private final WeakReference<ImageView> imageViewWeakRef;
        private boolean canceled = false;

        private LoadIconTask(ContainerFileManagerFragment fragment, ImageView imageView) {
            this.fragment = fragment;
            this.imageViewWeakRef = new WeakReference<>(imageView);
        }

        public void loadAsync(FileInfo file) {
            executor.execute(() -> {
                final Object icon = fragment.getIconForFile(file);
                final ImageView imageView = imageViewWeakRef.get();
                if (imageView != null && !canceled) {
                    imageView.post(() -> {
                        if (canceled) return;
                        if (icon instanceof Bitmap) {
                            imageView.setImageBitmap((Bitmap)icon);
                        }
                        else imageView.setImageResource((Integer)icon);
                    });
                }
            });
        }

        public void cancel() {
            canceled = true;
        }
    }

    private class FileInfoAdapter extends RecyclerView.Adapter<FileInfoAdapter.ViewHolder> {
        private final List<FileInfo> data;

        private class ViewHolder extends RecyclerView.ViewHolder {
            private final ImageView runButton;
            private final ImageView menuButton;
            private final ImageView imageView;
            private final TextView title;
            private final TextView subtitle;

            private ViewHolder(View view) {
                super(view);
                this.imageView = view.findViewById(R.id.ImageView);
                this.title = view.findViewById(R.id.TVTitle);
                this.subtitle = view.findViewById(R.id.TVSubtitle);
                this.runButton = view.findViewById(R.id.BTRun);
                this.menuButton = view.findViewById(R.id.BTMenu);
            }
        }

        public FileInfoAdapter(List<FileInfo> data) {
            this.data = data;
        }

        @NonNull
        @Override
        public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
            int resource = viewStyle == ViewStyle.LIST ? R.layout.file_list_item : R.layout.file_grid_item;
            return new ViewHolder(LayoutInflater.from(parent.getContext()).inflate(resource, parent, false));
        }

        @Override
        public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
            Context context = getContext();
            final FileInfo item = data.get(position);

            FileInfo.Type type = item.type;
            if (item.type == FileInfo.Type.DRIVE) {
                String driveText = getContext().getString(R.string.drive);
                holder.title.setText(driveText+" ("+item.name+")");
            }
            else {
                MSLink.LinkInfo linkInfo = item.getLinkinfo();
                if (linkInfo != null && linkInfo.isDirectory) type = FileInfo.Type.DIRECTORY;
                holder.title.setText(item.getDisplayName());
            }

            holder.subtitle.setVisibility(View.GONE);
            holder.runButton.setImageResource(R.drawable.icon_open);

            if (type == FileInfo.Type.DIRECTORY && !folderStack.isEmpty()) {
                holder.subtitle.setText(item.getItemCount()+" "+context.getString(R.string.items));
                holder.subtitle.setVisibility(View.VISIBLE);
            }
            else if (type == FileInfo.Type.FILE) {
                holder.runButton.setImageResource(R.drawable.icon_run);
                holder.subtitle.setText(StringUtils.formatBytes(item.getSize()));
                holder.subtitle.setVisibility(View.VISIBLE);
            }

            if (type == FileInfo.Type.FILE) {
                holder.imageView.setImageResource(R.drawable.container_file);
                LoadIconTask loadIconTask = (LoadIconTask)holder.imageView.getTag();
                if (loadIconTask != null) loadIconTask.cancel();

                loadIconTask = new LoadIconTask(ContainerFileManagerFragment.this, holder.imageView);
                loadIconTask.loadAsync(item);
                holder.imageView.setTag(loadIconTask);
            }
            else holder.imageView.setImageResource((Integer)getIconForFile(item));

            holder.imageView.setOnClickListener((v) -> openFile(item));
            holder.runButton.setOnClickListener((v) -> openFile(item));
            holder.menuButton.setOnClickListener((v) -> showListItemMenu(v, item));
        }

        @Override
        public final int getItemCount() {
            return data.size();
        }

        private void showListItemMenu(View anchorView, final FileInfo file) {
            final Context context = getContext();
            PopupMenu listItemMenu = new PopupMenu(context, anchorView);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) listItemMenu.setForceShowIcon(true);

            listItemMenu.inflate(R.menu.file_manager_popup_menu);

            Menu menu = listItemMenu.getMenu();
            menu.findItem(R.id.menu_item_settings).setVisible(false);

            if (folderStack.isEmpty()) {
                menu.findItem(R.id.menu_item_cut).setVisible(false);
                menu.findItem(R.id.menu_item_remove).setVisible(false);
                menu.findItem(R.id.menu_item_rename).setVisible(false);
                menu.findItem(R.id.menu_item_add_favorite).setVisible(false);
            }
            else if (folderStack.peek().name.equals("Favorites")) {
                menu.findItem(R.id.menu_item_add_favorite).setVisible(false);
            }

            listItemMenu.setOnMenuItemClickListener((menuItem) -> {
                int itemId = menuItem.getItemId();
                switch (itemId) {
                    case R.id.menu_item_copy:
                    case R.id.menu_item_cut:
                        instantiateClipboard(file, itemId == R.id.menu_item_cut);
                        break;
                    case R.id.menu_item_remove:
                        clearClipboard();
                        ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_file, () -> removeFile(file.toFile()));
                        break;
                    case R.id.menu_item_rename:
                        clearClipboard();
                        ContentDialog.prompt(context, R.string.rename, file.name, (newName) -> {
                            file.renameTo(newName);
                            refreshContent();
                        });
                        break;
                    case R.id.menu_item_add_favorite:
                        addFavorite(file);
                        break;
                    case R.id.menu_item_info:
                        (new FileInfoDialog(context, file, container)).show();
                        break;
                }
                return true;
            });
            listItemMenu.show();
        }

        private void openFile(FileInfo file) {
            Activity activity = getActivity();
            MSLink.LinkInfo linkInfo = file.getLinkinfo();
            boolean isFile = linkInfo != null ? !linkInfo.isDirectory : file.type == FileInfo.Type.FILE;

            if (isFile) {
                Intent intent = new Intent(activity, XServerDisplayActivity.class);
                intent.putExtra("container_id", container.id);
                intent.putExtra("exec_path", file.path);
                activity.startActivity(intent);
            }
            else {
                folderStack.push(file);
                refreshContent();
            }
        }
    }

    public Object getIconForFile(FileInfo file) {
        if (file.type == FileInfo.Type.DIRECTORY) {
            Context context = getContext();
            if (file.path.endsWith(RootFS.USER+"/"+context.getString(R.string.documents))) {
                return R.drawable.container_folder_documents;
            }
            else if (file.path.endsWith(RootFS.USER+"/"+context.getString(R.string.favorites))) {
                return R.drawable.container_folder_favorites;
            }
            else return R.drawable.container_folder;
        }
        else if (file.type == FileInfo.Type.DRIVE) {
            return R.drawable.container_drive;
        }
        else {
            String extension = FileUtils.getExtension(file.path);

            switch (extension) {
                case "exe": {
                    Bitmap bitmap = PEParser.extractIcon(file.toFile());
                    return bitmap != null ? bitmap : R.drawable.container_file_window;
                }
                case "bat": {
                    return R.drawable.container_file_window;
                }
                case "ico": {
                    Bitmap bitmap = MSIcon.decodeFile(file.toFile());
                    if (bitmap != null) return bitmap;
                    break;
                }
                case "dll":
                    return R.drawable.container_file_library;
                case "lnk": {
                    MSLink.LinkInfo linkInfo = file.getLinkinfo();
                    if (linkInfo != null) {
                        if (linkInfo.isDirectory) {
                            return R.drawable.container_folder;
                        }
                        else {
                            String targetPath = linkInfo.iconLocation != null ? linkInfo.iconLocation : linkInfo.targetPath;
                            targetPath = WineUtils.dosToUnixPath(targetPath, container);

                            Bitmap bitmap;
                            if (targetPath.endsWith(".ico")) {
                                bitmap = MSIcon.decodeFile(new File(targetPath));
                            }
                            else bitmap = PEParser.extractIcon(new File(targetPath), linkInfo.iconIndex);
                            if (bitmap != null) return bitmap;
                        }
                    }
                    return R.drawable.container_file_link;
                }
            }

            return R.drawable.container_file;
        }
    }
}
