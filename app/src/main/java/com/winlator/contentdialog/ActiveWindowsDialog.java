package com.winlator.contentdialog;

import android.graphics.Bitmap;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.winlator.R;
import com.winlator.XServerDisplayActivity;
import com.winlator.core.ImageUtils;
import com.winlator.core.UnitUtils;
import com.winlator.renderer.GLRenderer;
import com.winlator.xserver.Drawable;
import com.winlator.xserver.Window;
import com.winlator.xserver.XLock;
import com.winlator.xserver.XServer;

import java.util.ArrayList;

public class ActiveWindowsDialog extends ContentDialog {
    private final XServerDisplayActivity activity;

    public ActiveWindowsDialog(XServerDisplayActivity activity) {
        super(activity, R.layout.active_windows_dialog);
        this.activity = activity;
        setCancelable(false);
        setTitle(R.string.active_windows);
        setIcon(R.drawable.icon_active_windows);

        Button showDesktopButton = findViewById(R.id.BTCancel);
        showDesktopButton.setVisibility(View.VISIBLE);
        showDesktopButton.setText(R.string.show_desktop);
        showDesktopButton.setOnClickListener((v) -> {
            activity.getWinHandler().showDesktop();
            dismiss();
        });

        ArrayList<Window> windows = collectActiveWindows();
        loadWindowViews(windows);
    }

    private void loadWindowViews(ArrayList<Window> windows) {
        if (windows.isEmpty()) {
            findViewById(R.id.TVEmptyText).setVisibility(View.VISIBLE);
            return;
        }

        XServer xServer = activity.getXServer();
        LinearLayout llWindowList = findViewById(R.id.LLWindowList);
        llWindowList.removeAllViews();
        GLRenderer renderer = xServer.getRenderer();

        LayoutInflater inflater = LayoutInflater.from(activity);
        float iconSize = UnitUtils.dpToPx(24);
        int imageHeight = (int)UnitUtils.dpToPx(116);

        for (int i = windows.size()-1; i >= 0; i--) {
            final Window window = windows.get(i);
            Window parent = window.getParent();
            View itemView = inflater.inflate(R.layout.active_window_list_item, llWindowList, false);
            ImageView ivIcon = itemView.findViewById(R.id.IVIcon);
            final ImageView ivWindow = itemView.findViewById(R.id.IVWindow);
            TextView tvName = itemView.findViewById(R.id.TVName);

            String title = window.getName();
            if (title.isEmpty()) title = parent.getName();
            tvName.setText(title);

            Bitmap icon = xServer.pixmapManager.getWindowIcon(window);
            if (icon == null) icon = xServer.pixmapManager.getWindowIcon(parent);

            ivIcon.setImageResource(R.drawable.taskmgr_process);
            if (icon != null) ivIcon.setImageBitmap(icon);

            if (!window.isIconic()) {
                Drawable content = window.getContent();
                int[] scaledSize = ImageUtils.getScaledSize(content.width, content.height, 0, imageHeight);
                tvName.setMaxWidth((int)(scaledSize[0] - iconSize));
                ivWindow.setLayoutParams(new FrameLayout.LayoutParams(scaledSize[0], scaledSize[1]));

                renderer.takeWindowScreenshot(content, (bitmap) -> ivWindow.post(() -> ivWindow.setImageBitmap(bitmap)));
            }
            else {
                itemView.findViewById(R.id.IVHidden).setVisibility(View.VISIBLE);
                itemView.findViewById(R.id.IVDashedFrame).setVisibility(View.VISIBLE);
                tvName.setMaxWidth((int)(imageHeight - iconSize));
                ivWindow.setLayoutParams(new FrameLayout.LayoutParams(imageHeight, imageHeight));
            }

            ivWindow.setOnClickListener((v) -> {
                activity.getWinHandler().bringToFront(window.getClassName(), window.getHandle());
                dismiss();
            });

            llWindowList.addView(itemView);
        }
    }

    private void collectActiveWindows(Window window, ArrayList<Window> result) {
        if (!window.isRenderable()) return;

        XServer xServer = activity.getXServer();
        if ((window != xServer.windowManager.rootWindow && !window.isDesktopWindow() && !window.getName().isEmpty()) || window.isSurface()) {
            result.add(window);
        }

        for (Window child : window.getChildren()) {
            collectActiveWindows(child, result);
        }
    }

    private ArrayList<Window> collectActiveWindows() {
        XServer xServer = activity.getXServer();
        ArrayList<Window> result = new ArrayList<>();

        try (XLock lock = xServer.lock(XServer.Lockable.WINDOW_MANAGER, XServer.Lockable.DRAWABLE_MANAGER)) {
            collectActiveWindows(xServer.windowManager.rootWindow, result);
        }

        return result;
    }
}
