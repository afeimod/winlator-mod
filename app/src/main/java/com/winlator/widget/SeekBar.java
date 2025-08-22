package com.winlator.widget;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import android.os.Bundle;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.view.MotionEvent;

import androidx.appcompat.widget.AppCompatImageView;
import androidx.core.content.ContextCompat;

import com.winlator.R;
import com.winlator.core.StringUtils;
import com.winlator.core.UnitUtils;
import com.winlator.math.Mathf;

import java.text.DecimalFormat;

public class SeekBar extends AppCompatImageView {
    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private DecimalFormat decimalFormat;
    private float padding;
    private float minValue = 0;
    private float maxValue = 100;
    private float step = 1;
    private float normalizedValue = 0;
    private OnValueChangeListener onValueChangeListener;
    private float textSize = UnitUtils.dpToPx(16);
    private final int textColor = 0xff737373;
    private final float barHeight = UnitUtils.dpToPx(6);
    private final float thumbSize = UnitUtils.dpToPx(20);
    private final float thumbRadius;
    private final RectF rect = new RectF();
    private String suffix;
    private final int colorPrimary = 0xffd7d7d7;
    private final int colorSecondary;
    private LinearGradient glossyEffectGradient;

    public interface OnValueChangeListener {
        void onValueChangeListener(SeekBar seekBar, float value);
    }

    public SeekBar(Context context) {
        this(context, null);
    }

    public SeekBar(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public SeekBar(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        colorSecondary = ContextCompat.getColor(context, R.color.colorAccent);

        if (attrs != null) {
            TypedArray ta = context.obtainStyledAttributes(attrs, R.styleable.SeekBar, 0, 0);
            minValue = ta.getFloat(R.styleable.SeekBar_minValue, minValue);
            maxValue = ta.getFloat(R.styleable.SeekBar_maxValue, maxValue);
            suffix = ta.getString(R.styleable.SeekBar_suffix);
            textSize = ta.getDimension(R.styleable.SeekBar_textSize, textSize);

            setStep(ta.getFloat(R.styleable.SeekBar_step, step));
            setValue(ta.getFloat(R.styleable.SeekBar_value, 0));
            ta.recycle();
        }

        thumbRadius = thumbSize / 2.0f;
        setFocusable(true);
        setFocusableInTouchMode(true);
    }

    public String getSuffix() {
        return suffix;
    }

    public synchronized void setSuffix(String suffix) {
        this.suffix = suffix;
    }

    public OnValueChangeListener getOnValueChangeListener() {
        return onValueChangeListener;
    }

    public void setOnValueChangeListener(OnValueChangeListener onValueChangeListener) {
        this.onValueChangeListener = onValueChangeListener;
    }

    public float getValue() {
        return minValue + normalizedValue * (maxValue - minValue);
    }

    public synchronized void setValue(float value) {
        normalizedValue = Mathf.clamp((Mathf.roundTo(value, step) - minValue) / (maxValue - minValue), 0.0f, 1.0f);
        postInvalidate();
    }

    public float getStep() {
        return step;
    }

    public synchronized void setStep(float step) {
        this.step = step;
        String[] parts = String.valueOf(Mathf.fract(step)).split("\\.");
        int decimalPlaces = parts[parts.length-1].length();
        String format = "0."+ StringUtils.repeat(decimalPlaces > 1 ? '0' : '#', decimalPlaces);
        decimalFormat = new DecimalFormat(format);
    }

    public float getMinValue() {
        return minValue;
    }

    public synchronized void setMinValue(float minValue) {
        this.minValue = minValue;
    }

    public float getMaxValue() {
        return maxValue;
    }

    public synchronized void setMaxValue(float maxValue) {
        this.maxValue = maxValue;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!isEnabled()) return false;

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN :
                setPressed(isInThumbRange(event.getX()));
                if (!isPressed()) return super.onTouchEvent(event);

                setNormalizedValue(event.getX());
                if (getParent() != null) getParent().requestDisallowInterceptTouchEvent(true);
                invalidate();
                break;
            case MotionEvent.ACTION_MOVE :
                if (isPressed()) {
                    setNormalizedValue(event.getX());
                    invalidate();
                }
                break;
            case MotionEvent.ACTION_UP :
                if (isPressed()) {
                    setNormalizedValue(event.getX());
                    setPressed(false);
                }
                invalidate();
                if (onValueChangeListener != null) onValueChangeListener.onValueChangeListener(this, getValue());
                break;
            case MotionEvent.ACTION_CANCEL :
                if (isPressed()) setPressed(false);
                invalidate();
                break;
        }
        return true;
    }

    @Override
    protected synchronized void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int margin = (int)UnitUtils.dpToPx(2);
        int width = (int)UnitUtils.dpToPx(220);
        int height = (int)(thumbSize + margin);
        setMeasuredDimension(resolveSizeAndState(width + margin, widthMeasureSpec, 0), resolveSizeAndState(height, heightMeasureSpec, 0));
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        int width = getWidth();
        int height = getHeight();
        if (width == 0 || height == 0) return;

        float centerY = height * 0.5f;
        paint.setTextSize(textSize);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(textColor);
        paint.setAntiAlias(true);

        String text = decimalFormat.format(getValue()) + (suffix != null ? suffix : "");
        int repeatCount = 4 + (Mathf.fract(step) > 0 || minValue < 0 ? 1 : 0) + (suffix != null ? suffix.length() : 0);
        float textWidth = paint.measureText(StringUtils.repeat('0', repeatCount));
        canvas.drawText(text, width - paint.measureText(text), centerY + textSize / 3, paint);
        padding = textWidth + thumbSize;
        float screenCoord = getScreenCoord();
        rect.set(0, centerY - barHeight * 0.5f, width - textWidth, centerY + barHeight * 0.5f);

        paint.setColor(colorPrimary);

        float cornerRadius = rect.height() * 0.5f;
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, paint);
        paint.setShader(null);

        rect.right = screenCoord;
        paint.setColor(colorSecondary);
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, paint);

        rect.right = width - textWidth;
        if (glossyEffectGradient == null) {
            glossyEffectGradient = new LinearGradient(0, 0, 0, height, new int[]{0x33ffffff, 0x00000000}, new float[]{0.5f, 0.5f}, Shader.TileMode.CLAMP);
        }
        paint.setShader(glossyEffectGradient);
        canvas.drawRoundRect(rect, cornerRadius, cornerRadius, paint);
        paint.setShader(null);

        canvas.drawCircle(screenCoord, centerY, thumbRadius, paint);
        paint.setColor(getThumbHoleColor());
        canvas.drawCircle(screenCoord, centerY, thumbRadius * 0.5f, paint);
    }

    private int getThumbHoleColor() {
        int r = Mathf.clamp(Color.red(colorSecondary) - 30, 0, 255);
        int g = Mathf.clamp(Color.green(colorSecondary) - 30, 0, 255);
        int b = Mathf.clamp(Color.blue(colorSecondary) - 30, 0, 255);
        return Color.rgb(r, g, b);
    }

    @Override
    protected Parcelable onSaveInstanceState() {
        final Bundle bundle = new Bundle();
        bundle.putParcelable("SUPER", super.onSaveInstanceState());
        bundle.putFloat("normalizedValue", normalizedValue);
        return bundle;
    }

    @Override
    protected void onRestoreInstanceState(Parcelable parcel) {
        final Bundle bundle = (Bundle) parcel;
        super.onRestoreInstanceState(bundle.getParcelable("SUPER"));
        normalizedValue = bundle.getFloat("normalizedValue");
    }

    private boolean isInThumbRange(float touchX) {
        return Math.abs(touchX - getScreenCoord()) <= thumbRadius;
    }

    private float getScreenCoord() {
        return thumbRadius + normalizedValue * (getWidth() - padding);
    }

    private void setNormalizedValue(float touchX) {
        int width = getWidth();
        if (width - padding <= 0) return;
        float normalizedStep = step / (maxValue - minValue);
        normalizedValue = Mathf.clamp(Mathf.roundTo((touchX - thumbRadius) / (width - padding), normalizedStep), 0.0f, 1.0f);
    }
}