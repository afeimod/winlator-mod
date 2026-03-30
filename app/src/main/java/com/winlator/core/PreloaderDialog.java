package com.winlator.core;

import android.app.Activity;
import android.app.Dialog;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.winlator.R;

public class PreloaderDialog {
    private final Activity activity;
    private Dialog dialog;

    public PreloaderDialog(Activity activity) {
        this.activity = activity;
    }

    private void create() {
        if (dialog != null || isActivityDestroyed()) return;
        dialog = new Dialog(activity, android.R.style.Theme_Translucent_NoTitleBar_Fullscreen);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setContentView(R.layout.preloader_dialog);

        Window window = dialog.getWindow();
        if (window != null) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
            window.clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
        }
    }

    private boolean isActivityDestroyed() {
        return activity == null || activity.isFinishing() || activity.isDestroyed();
    }

    public synchronized void show(int textResId) {
        if (isActivityDestroyed() || isShowing()) return;
        close();
        create();
        if (dialog != null) {
            ((TextView)dialog.findViewById(R.id.TextView)).setText(textResId);
            dialog.show();
        }
    }

    public void showOnUiThread(final int textResId) {
        if (isActivityDestroyed()) return;
        activity.runOnUiThread(() -> show(textResId));
    }

    public synchronized void close() {
        try {
            if (dialog != null && dialog.isShowing()) {
                dialog.dismiss();
            }
        }
        catch (Exception e) {}
        finally {
            dialog = null;
        }
    }

    public void closeOnUiThread() {
        if (isActivityDestroyed()) return;
        activity.runOnUiThread(this::close);
    }

    public boolean isShowing() {
        return dialog != null && dialog.isShowing();
    }
}
