package com.winlator.widget;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.ArraySet;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.winlator.core.UnitUtils;

public class SimplePianoKeyboard extends View {
    private final float SHARP_KEY_SCALE = 0.6f;
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final Path path = new Path();
    private int octaves = 1;
    private OnKeyListener onKeyListener;
    private final ArraySet<Byte> pressedKeys = new ArraySet<>();

    public interface OnKeyListener {
        void onKeyDown(int index);
        void onKeyUp(int index);
    }

    public SimplePianoKeyboard(Context context) {
        this(context, null);
    }

    public SimplePianoKeyboard(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public SimplePianoKeyboard(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public int getOctaves() {
        return octaves;
    }

    public void setOctaves(int octaves) {
        this.octaves = octaves;
    }

    public OnKeyListener getOnKeyListener() {
        return onKeyListener;
    }

    public void setOnKeyListener(OnKeyListener onKeyListener) {
        this.onKeyListener = onKeyListener;
    }

    @Override
    protected void onDraw(@NonNull Canvas canvas) {
        super.onDraw(canvas);

        int width = getWidth();
        int height = getHeight();
        if (width == 0 || height == 0) return;

        float strokeWidth = UnitUtils.dpToPx(2);
        float keyWidth = (float)width / (octaves * 7);
        float sharpKeyWidth = keyWidth * SHARP_KEY_SCALE;
        float sharpKeyBottom = height * SHARP_KEY_SCALE;
        paint.setStrokeWidth(strokeWidth);
        float radius = UnitUtils.dpToPx(6);
        float[] bottomRadius = {0.0f, 0.0f, 0.0f, 0.0f, radius, radius, radius, radius};

        float offsetX = strokeWidth * 0.5f;
        float startY = strokeWidth * 0.5f;
        float endY = height - strokeWidth * 0.5f;
        byte index = 0;
        boolean sharpKeyPressed;
        for (int i = 0, j; i < octaves; i++) {
            for (j = 0; j < 7; j++) {
                sharpKeyPressed = false;
                if (j != 3 && j != 0) {
                    sharpKeyPressed = pressedKeys.contains(index);
                    index++;
                }

                path.reset();
                path.addRoundRect(offsetX, startY, offsetX + keyWidth, endY, bottomRadius, Path.Direction.CW);

                if (pressedKeys.contains(index)) {
                    paint.setStyle(Paint.Style.FILL);
                    paint.setColor(0xffc0c0c0);
                    canvas.drawPath(path, paint);
                }

                paint.setStyle(Paint.Style.STROKE);
                paint.setColor(0xff000000);
                canvas.drawPath(path, paint);
                index++;

                if (j != 3 && j != 0) {
                    path.reset();
                    path.addRoundRect(offsetX - sharpKeyWidth * 0.5f, startY, offsetX + sharpKeyWidth * 0.5f, sharpKeyBottom, bottomRadius, Path.Direction.CW);
                    paint.setStyle(Paint.Style.FILL);
                    paint.setColor(sharpKeyPressed ? 0xff606060 : 0xff000000);
                    canvas.drawPath(path, paint);

                    if (sharpKeyPressed) {
                        paint.setStyle(Paint.Style.STROKE);
                        paint.setColor(0xff000000);
                        canvas.drawPath(path, paint);
                    }
                }

                offsetX += keyWidth - strokeWidth * 0.5f;
            }
        }
    }

    private byte keyIndexFromTouchPoint(float touchX, float touchY) {
        int width = getWidth();
        int height = getHeight();

        float keyWidth = (float)width / (octaves * 7);
        float sharpKeyWidth = keyWidth * SHARP_KEY_SCALE;
        float sharpKeyBottom = height * SHARP_KEY_SCALE;

        float offsetX = 0;
        byte index = 0;
        for (int i = 0, j; i < octaves; i++) {
            for (j = 0; j < 7; j++) {
                if (j != 3 && j != 0) {
                    if (touchX >= offsetX - sharpKeyWidth * 0.5f && touchX <= offsetX + sharpKeyWidth * 0.5f && touchY <= sharpKeyBottom) {
                        return index;
                    }
                    index++;
                }

                if (touchX >= offsetX && touchX <= offsetX + keyWidth) return index;
                index++;
                offsetX += keyWidth;
            }
        }

        return -1;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getAction();
        if (action == MotionEvent.ACTION_DOWN) {
            byte index = keyIndexFromTouchPoint(event.getX(), event.getY());
            if (index != -1) {
                if (onKeyListener != null) onKeyListener.onKeyDown(index);
                pressedKeys.add(index);
                postInvalidate();
                return true;
            }
        }
        else if (action == MotionEvent.ACTION_UP) {
            if (onKeyListener != null) for (int index : pressedKeys) onKeyListener.onKeyUp(index);
            pressedKeys.clear();
            postInvalidate();
            return true;
        }

        return super.onTouchEvent(event);
    }
}
