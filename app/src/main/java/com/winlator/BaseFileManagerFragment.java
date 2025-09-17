package com.winlator;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.preference.PreferenceManager;
import androidx.recyclerview.widget.DividerItemDecoration;
import androidx.recyclerview.widget.GridLayoutManager;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.winlator.container.ContainerManager;
import com.winlator.core.AppUtils;
import com.winlator.core.FileUtils;
import com.winlator.core.PreloaderDialog;
import com.winlator.core.UnitUtils;

import java.io.File;
import java.util.Stack;
import java.util.concurrent.Executors;

public abstract class BaseFileManagerFragment<T> extends Fragment {
    protected enum ViewStyle {LIST, GRID};
    protected PreloaderDialog preloaderDialog;
    protected RecyclerView recyclerView;
    protected TextView emptyTextView;
    protected FloatingActionButton pasteButton;
    protected ViewStyle viewStyle = ViewStyle.GRID;
    protected boolean viewStyleNeedsUpdate = true;
    protected DividerItemDecoration itemDecoration;
    protected SharedPreferences preferences;
    protected Clipboard clipboard;
    protected ContainerManager manager;
    protected final Stack<T> folderStack = new Stack<>();

    static class Clipboard {
        File targetDir;
        final File[] files;
        final boolean cutMode;

        Clipboard(File[] files, boolean cutMode) {
            this.files = files;
            this.cutMode = cutMode;
        }
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);

        Activity activity = getActivity();
        preloaderDialog = new PreloaderDialog(activity);
        preferences = PreferenceManager.getDefaultSharedPreferences(activity);
        manager = new ContainerManager(activity);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.base_file_manager_fragment, container, false);
        recyclerView = rootView.findViewById(R.id.RecyclerView);
        emptyTextView = rootView.findViewById(R.id.TVEmptyText);

        pasteButton = rootView.findViewById(R.id.BTPaste);
        pasteButton.setOnClickListener((v) -> pasteFiles());

        if (itemDecoration == null) {
            Context context = getContext();
            itemDecoration = new DividerItemDecoration(context, DividerItemDecoration.VERTICAL);
            itemDecoration.setDrawable(ContextCompat.getDrawable(context, R.drawable.list_item_divider));
        }
        return rootView;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        ((AppCompatActivity)getActivity()).getSupportActionBar().setTitle(getHomeTitle());
        refreshContent();
    }

    public void setViewStyle(ViewStyle viewStyle) {
        this.viewStyle = viewStyle;
        viewStyleNeedsUpdate = true;
        refreshContent();
    }

    public boolean onBackPressed() {
        clearClipboard();
        return onOptionsMenuClicked();
    }

    public boolean onOptionsMenuClicked() {
        if (!folderStack.isEmpty()) {
            folderStack.pop();
            refreshContent();

            if (folderStack.isEmpty()) {
                ActionBar actionBar = ((AppCompatActivity)getActivity()).getSupportActionBar();
                actionBar.setHomeAsUpIndicator(R.drawable.icon_action_bar_menu);
                actionBar.setTitle(getHomeTitle());
            }
            return true;
        }
        else return false;
    }

    public void onOrientationChanged() {
        viewStyleNeedsUpdate = true;
        refreshContent();
    }

    public void refreshViewStyleMenuItem(MenuItem menuItem) {
        if (viewStyle == ViewStyle.LIST) {
            menuItem.setIcon(R.drawable.icon_action_bar_grid);
        }
        else if (viewStyle == ViewStyle.GRID) {
            menuItem.setIcon(R.drawable.icon_action_bar_list);
        }
    }

    public void refreshContent() {
        if (viewStyleNeedsUpdate) {
            Context context = getContext();
            recyclerView.removeItemDecoration(itemDecoration);
            if (viewStyle == ViewStyle.LIST) {
                recyclerView.setLayoutManager(new LinearLayoutManager(context));
                recyclerView.addItemDecoration(itemDecoration);
            }
            else if (viewStyle == ViewStyle.GRID) {
                int spanCount = Math.max(2, (int)(AppUtils.getScreenWidth() / UnitUtils.dpToPx(200)));
                recyclerView.setLayoutManager(new GridLayoutManager(context, spanCount));
            }
            viewStyleNeedsUpdate = false;
        }
    }

    protected void pasteFiles() {
        if (clipboard == null) return;
        FragmentActivity activity = getActivity();

        for (File file : clipboard.files) {
            File targetFile = new File(clipboard.targetDir, file.getName());
            if (targetFile.exists()) {
                AppUtils.showToast(activity, R.string.there_already_file_with_that_name);
                return;
            }
        }

        preloaderDialog.show(R.string.copying_files);
        Executors.newSingleThreadExecutor().execute(() -> {
            for (File originFile : clipboard.files) {
                if (originFile.exists()) {
                    File targetFile = new File(clipboard.targetDir, originFile.getName());
                    if (FileUtils.copy(originFile, targetFile) && clipboard.cutMode) FileUtils.delete(originFile);
                }
            }

            activity.runOnUiThread(() -> {
                clearClipboard();
                refreshContent();
                preloaderDialog.close();
            });
        });
    }

    protected void removeFile(final File file) {
        preloaderDialog.show(R.string.removing_files);
        Executors.newSingleThreadExecutor().execute(() -> {
            FileUtils.delete(file);

            getActivity().runOnUiThread(() -> {
                clearClipboard();
                refreshContent();
                preloaderDialog.close();
            });
        });
    }

    public void clearClipboard() {
        if (clipboard != null) {
            clipboard = null;
            pasteButton.setVisibility(View.GONE);
        }
    }

    protected abstract String getHomeTitle();
}
