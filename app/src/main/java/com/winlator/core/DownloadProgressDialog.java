package com.winlator.core;

import android.app.Activity;
import android.app.Dialog;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;

import com.google.android.material.progressindicator.CircularProgressIndicator;
import com.winlator.R;
import com.winlator.math.Mathf;

public class DownloadProgressDialog {
    private final Activity activity;
    private Dialog dialog;

    public DownloadProgressDialog(Activity activity) {
        this.activity = activity;
    }

    private void create() {
        if (dialog != null) return;
        dialog = new Dialog(activity, android.R.style.Theme_Translucent_NoTitleBar_Fullscreen);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.setContentView(R.layout.download_progress_dialog);

        Window window = dialog.getWindow();
        if (window != null) {
            window.clearFlags(WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE);
            window.clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
        }
    }

    public void show() {
        show(0, null);
    }

    public void show(int textResId) {
        show(textResId, null);
    }

    public void show(Runnable onCancelCallback) {
        show(0, onCancelCallback);
    }

    public void show(int textResId, final Runnable onCancelCallback) {
        activity.runOnUiThread(() -> {
            if (isShowing()) {
                if (textResId > 0) setText(textResId);
                setCancelCallback(onCancelCallback);
                return;
            }
            close();
            if (dialog == null) create();

            if (textResId > 0) ((TextView)dialog.findViewById(R.id.TextView)).setText(textResId);

            setProgress(0);
            setCancelCallback(onCancelCallback);
            dialog.show();
        });
    }

    public void setText(int textResId) {
        activity.runOnUiThread(() -> {
            if (dialog != null && textResId > 0) {
                ((TextView)dialog.findViewById(R.id.TextView)).setText(textResId);
            }
        });
    }

    public void setCancelCallback(Runnable onCancelCallback) {
        activity.runOnUiThread(() -> {
            if (dialog == null) return;
            View cancelBtn = dialog.findViewById(R.id.BTCancel);
            View bottomBar = dialog.findViewById(R.id.LLBottomBar);
            if (onCancelCallback != null) {
                cancelBtn.setOnClickListener((v) -> onCancelCallback.run());
                bottomBar.setVisibility(View.VISIBLE);
            } else {
                bottomBar.setVisibility(View.GONE);
            }
        });
    }

    public void setProgress(int progress) {
        activity.runOnUiThread(() -> {
            if (dialog == null) return;
            int p = Mathf.clamp(progress, 0, 100);
            ((CircularProgressIndicator)dialog.findViewById(R.id.CircularProgressIndicator)).setProgress(p);
            ((TextView)dialog.findViewById(R.id.TVProgress)).setText(p+"%");
        });
    }

    public void close() {
        try {
            if (dialog != null) {
                dialog.dismiss();
            }
        }
        catch (Exception e) {}
    }

    public void closeOnUiThread() {
        activity.runOnUiThread(this::close);
    }

    public boolean isShowing() {
        return dialog != null && dialog.isShowing();
    }
}
