package com.termux.x11;

import android.annotation.SuppressLint;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.inputmethod.InputMethodManager;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.preference.PreferenceManager;

import static java.nio.charset.StandardCharsets.UTF_8;

import dalvik.annotation.optimization.CriticalNative;
import dalvik.annotation.optimization.FastNative;

@Keep @SuppressLint("WrongConstant")
@SuppressWarnings("deprecation")
public class LorieView extends SurfaceView {
    public interface Callback {
        void changed(int surfaceWidth, int surfaceHeight, int screenWidth, int screenHeight);
    }

    interface PixelFormat {
        int BGRA_8888 = 5; // Stands for HAL_PIXEL_FORMAT_BGRA_8888
    }

    private ClipboardManager clipboard;
    private long lastClipboardTimestamp = System.currentTimeMillis();
    private static boolean clipboardSyncEnabled = false;
    private static boolean hardwareKbdScancodesWorkaround = false;
    private final InputMethodManager mIMM = (InputMethodManager)getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
    private Callback mCallback;
    /** 桌面分辨率，在 MainActivity.onCreate 中设置 */
    public final Point p = new Point();
    private SharedPreferences preferences;
    private boolean toggleFullscreen = false;
    boolean commitedText = false;
    private final SurfaceHolder.Callback mSurfaceCallback = new SurfaceHolder.Callback() {
        @Override public void surfaceCreated(@NonNull SurfaceHolder holder) {
            holder.setFormat(PixelFormat.BGRA_8888);
        }

        @Override public void surfaceChanged(@NonNull SurfaceHolder holder, int f, int width, int height) {
            LorieView.this.surfaceChanged(holder.getSurface());
            width = getMeasuredWidth();
            height = getMeasuredHeight();

            Log.d("SurfaceChangedListener", "Surface was changed: " + width + "x" + height);
            if (mCallback != null)
                mCallback.changed(width, height, p.x, p.y);
        }

        @Override public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
            LorieView.this.surfaceChanged(null);
            if (mCallback != null)
                mCallback.changed(0, 0, 0, 0);
        }
    };

    public LorieView(Context context) { super(context); init(); }
    public LorieView(Context context, AttributeSet attrs) { super(context, attrs); init(); }
    public LorieView(Context context, AttributeSet attrs, int defStyleAttr) { super(context, attrs, defStyleAttr); init(); }
    @SuppressWarnings("unused")
    public LorieView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) { super(context, attrs, defStyleAttr, defStyleRes); init(); }

    private void init() {
        preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        getHolder().addCallback(mSurfaceCallback);
        clipboard = (ClipboardManager) getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        nativeInit();
    }

    public void setCallback(Callback callback) {
        mCallback = callback;
        triggerCallback();
    }

    public void triggerCallback() {
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();

        setBackground(new ColorDrawable(Color.TRANSPARENT) {
            public boolean isStateful() {
                return true;
            }
            public boolean hasFocusStateSpecified() {
                return true;
            }
        });

        Rect r = getHolder().getSurfaceFrame();
        MainActivity.getInstance().runOnUiThread(() -> mSurfaceCallback.surfaceChanged(getHolder(), PixelFormat.BGRA_8888, r.width(), r.height()));
    }

    // TODO 适配宽高
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);

        if (toggleFullscreen) {
            getHolder().setSizeFromLayout();
            return;
        }

        if (p.x <= 0 || p.y <= 0)
            return;

        int width = getMeasuredWidth();
        int height = getMeasuredHeight();

        if (width > height * p.x / p.y)
            width = height * p.x / p.y;
        else
            height = width * p.y / p.x;

        getHolder().setFixedSize(p.x, p.y);
        setMeasuredDimension(width, height);
    }

    public void toggleFullscreen() {
        toggleFullscreen = !toggleFullscreen;
        requestLayout();
    }

    public boolean isToggleFullscreen() {
        return toggleFullscreen;
    }

    // It is used in native code
    void setClipboardText(String text) {
        clipboard.setPrimaryClip(ClipData.newPlainText("X11 clipboard", text));

        // Android does not send PrimaryClipChanged event to the window which posted event
        // But in the case we are owning focus and clipboard is unchanged it will be replaced by the same value on X server side.
        // Not cool in the case if user installed some clipboard manager, clipboard content will be doubled.
        lastClipboardTimestamp = System.currentTimeMillis() + 150;
    }

    /** @noinspection unused*/ // It is used in native code
    void requestClipboard() {
        if (!clipboardSyncEnabled) {
            sendClipboardEvent("".getBytes(UTF_8));
            return;
        }

        CharSequence clip = clipboard.getText();
        if (clip != null) {
            String text = String.valueOf(clipboard.getText());
            sendClipboardEvent(text.getBytes(UTF_8));
            Log.d("CLIP", "sending clipboard contents: " + text);
        }
    }

    /**
     * Unfortunately there is no direct way to focus inside X windows.
     * As a workaround we will reset IME on X window focus change and any user interaction
     * with LorieView except sending keys, text (Unicode) and mouse movements.
     * We must reset IME to get rid of pending composing, predictive text and other status related stuff.
     * It is called from native code, not from Java.
     * @noinspection unused
     */
    @Keep void resetIme() {
        if (!commitedText)
            return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU)
            mIMM.invalidateInput(this);
        else
            mIMM.restartInput(this);
    }

    @FastNative private native void nativeInit();
    @FastNative private native void surfaceChanged(Surface surface);
    @FastNative static native void connect(int fd);
    @CriticalNative static native boolean connected();
    @FastNative static native void startLogcat(int fd);
    @FastNative static native void setClipboardSyncEnabled(boolean enabled, boolean ignored);
    @FastNative public native void sendClipboardAnnounce();
    @FastNative public native void sendClipboardEvent(byte[] text);
    @FastNative static native void sendWindowChange(int width, int height, int framerate, String name);
    @FastNative public native void sendMouseEvent(float x, float y, int whichButton, boolean buttonDown, boolean relative);
    @FastNative public native void sendTouchEvent(int action, int id, int x, int y);
    @FastNative public native void sendStylusEvent(float x, float y, int pressure, int tiltX, int tiltY, int orientation, int buttons, boolean eraser, boolean mouseMode);
    @FastNative static public native void requestStylusEnabled(boolean enabled);
    public boolean sendKeyEvent(int scanCode, int keyCode, boolean keyDown) {
//        if (keyCode == 67)
//            new Exception().printStackTrace();
        return sendKeyEvent(scanCode, keyCode, keyDown, 0);
    }
    @FastNative public native boolean sendKeyEvent(int scanCode, int keyCode, boolean keyDown, int a);
    @FastNative public native void sendTextEvent(byte[] text);
    @CriticalNative public static native boolean requestConnection();

    static {
        System.loadLibrary("Xlorie");
    }
}
