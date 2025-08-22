package com.winlator;

import android.content.Context;
import android.content.Intent;
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

import com.winlator.container.Shortcut;
import com.winlator.contentdialog.ContentDialog;
import com.winlator.contentdialog.CreateFolderDialog;
import com.winlator.contentdialog.ShortcutSettingsDialog;
import com.winlator.core.AppUtils;
import com.winlator.core.ArrayUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ShortcutsFragment extends BaseFileManagerFragment<Shortcut> {
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        viewStyle = ViewStyle.valueOf(preferences.getString("shortcuts_view_style", "GRID"));
    }

    @Override
    public void refreshContent() {
        super.refreshContent();

        Shortcut selectedFolder = !folderStack.isEmpty() ? folderStack.peek() : null;
        ArrayList<Shortcut> shortcuts = manager.loadShortcuts(selectedFolder);
        recyclerView.setAdapter(new ShortcutsAdapter(shortcuts));
        emptyTextView.setVisibility(shortcuts.isEmpty() ? View.VISIBLE : View.GONE);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater menuInflater) {
        menuInflater.inflate(R.menu.shortcuts_menu, menu);
        refreshViewStyleMenuItem(menu.findItem(R.id.menu_item_view_style));
    }

    private void createFolder() {
        clearClipboard();
        if (manager.getContainers().isEmpty()) return;
        CreateFolderDialog createFolderDialog = new CreateFolderDialog(manager);
        createFolderDialog.setOnCreateFolderListener((container, name) -> {
            File desktopDir = new File(container.getUserDir(), "Desktop");
            File parent = !folderStack.isEmpty() ? folderStack.peek().file : desktopDir;
            File file = new File(parent, name);
            if (file.isDirectory()) {
                AppUtils.showToast(getContext(), R.string.there_already_file_with_that_name);
            }
            else {
                file.mkdir();
                refreshContent();
            }
        });
        createFolderDialog.show();
    }

    @Override
    protected void pasteFiles() {
        if (folderStack.isEmpty()) {
            clearClipboard();
            AppUtils.showToast(getContext(), R.string.you_cannot_paste_files_here);
            return;
        }

        clipboard.targetDir = folderStack.peek().file;
        super.pasteFiles();
    }

    private void instantiateClipboard(Shortcut shortcut, boolean cutMode) {
        clearClipboard();
        File linkFile = shortcut.getLinkFile();
        File[] files = {new File(shortcut.file.getParentFile(), shortcut.file.getName())};
        if (shortcut.file.isFile()) files = ArrayUtils.concat(files, new File[]{new File(linkFile.getParentFile(), linkFile.getName())});

        clipboard = new Clipboard(files, cutMode);
        pasteButton.setVisibility(View.VISIBLE);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem menuItem) {
        int itemId = menuItem.getItemId();
        if (itemId == R.id.menu_item_view_style) {
            setViewStyle(viewStyle == ViewStyle.GRID ? ViewStyle.LIST : ViewStyle.GRID);
            preferences.edit().putString("shortcuts_view_style", viewStyle.name()).apply();
            refreshViewStyleMenuItem(menuItem);
            return true;
        }
        else if (itemId == R.id.menu_item_new_folder) {
            createFolder();
            return true;
        }
        else return super.onOptionsItemSelected(menuItem);
    }

    @Override
    protected String getHomeTitle() {
        return getString(R.string.shortcuts);
    }

    private class ShortcutsAdapter extends RecyclerView.Adapter<ShortcutsAdapter.ViewHolder> {
        private final List<Shortcut> data;

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

        public ShortcutsAdapter(List<Shortcut> data) {
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
            final Shortcut item = data.get(position);

            if (item.icon == null) {
                int iconResId = item.file.isDirectory() ? R.drawable.container_folder : R.drawable.container_file_link;
                holder.imageView.setImageResource(iconResId);
            }
            else holder.imageView.setImageBitmap(item.icon);

            holder.title.setText(item.name);
            holder.subtitle.setText(item.container.getName());

            if (item.file.isDirectory()) {
                holder.runButton.setImageResource(R.drawable.icon_open);
            }
            else holder.runButton.setImageResource(R.drawable.icon_run);

            holder.imageView.setOnClickListener((v) -> runFromShortcut(item));
            holder.runButton.setOnClickListener((v) -> runFromShortcut(item));
            holder.menuButton.setOnClickListener((v) -> showListItemMenu(v, item));
        }

        @Override
        public final int getItemCount() {
            return data.size();
        }

        private void showListItemMenu(View anchorView, final Shortcut shortcut) {
            final Context context = getContext();
            PopupMenu listItemMenu = new PopupMenu(context, anchorView);
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) listItemMenu.setForceShowIcon(true);

            listItemMenu.inflate(R.menu.file_manager_popup_menu);

            Menu menu = listItemMenu.getMenu();
            menu.findItem(R.id.menu_item_rename).setVisible(false);
            menu.findItem(R.id.menu_item_add_favorite).setVisible(false);
            menu.findItem(R.id.menu_item_info).setVisible(false);

            listItemMenu.setOnMenuItemClickListener((menuItem) -> {
                int itemId = menuItem.getItemId();
                switch (itemId) {
                    case R.id.menu_item_settings:
                        clearClipboard();
                        (new ShortcutSettingsDialog(ShortcutsFragment.this, shortcut)).show();
                        break;
                    case R.id.menu_item_copy:
                    case R.id.menu_item_cut:
                        instantiateClipboard(shortcut, itemId == R.id.menu_item_cut);
                        break;
                    case R.id.menu_item_remove:
                        clearClipboard();
                        ContentDialog.confirm(context, R.string.do_you_want_to_remove_this_file, () -> {
                            shortcut.remove();
                            refreshContent();
                        });
                        break;
                }
                return true;
            });
            listItemMenu.show();
        }

        private void runFromShortcut(Shortcut shortcut) {
            AppCompatActivity activity = (AppCompatActivity)getActivity();

            if (shortcut.file.isDirectory()) {
                folderStack.push(shortcut);
                refreshContent();

                ActionBar actionBar = activity.getSupportActionBar();
                actionBar.setHomeAsUpIndicator(R.drawable.icon_action_bar_back);
                actionBar.setTitle(shortcut.name);
            }
            else {
                Intent intent = new Intent(activity, XServerDisplayActivity.class);
                intent.putExtra("container_id", shortcut.container.id);
                intent.putExtra("shortcut_path", shortcut.file.getPath());
                activity.startActivity(intent);
            }
        }
    }
}
