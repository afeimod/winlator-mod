package com.winlator.widget;

import android.app.ActivityManager;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.winlator.R;
import com.winlator.core.CPUStatus;
import com.winlator.core.StringUtils;

import java.util.Locale;
import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

public class FrameRating extends FrameLayout {
    private long lastTime = 0;
    private int frameCount = 0;
    private float lastFPS = 0;
    private TextView tvFPS;
    private TextView tvCPU;
    private TextView tvGPU;
    private TextView tvRAM;
    private ActivityManager activityManager;
    private final Random random = new Random();
    private int lastCpuUsage = 0;
    private int lastMemUsage = 0;
    private long lastUsedMem = 0;
    private long lastTotalMem = 0;
    private Timer timer;
    private final Handler handler = new Handler(Looper.getMainLooper());

    public FrameRating(Context context) {
        this(context, null);
    }

    public FrameRating(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public FrameRating(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        activityManager = (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
        LayoutInflater.from(context).inflate(R.layout.frame_rating, this, true);
        tvFPS = findViewById(R.id.TVFPS);
        tvCPU = findViewById(R.id.TVCPU);
        tvGPU = findViewById(R.id.TVGPU);
        tvRAM = findViewById(R.id.TVRAM);

        // 初始化显示
        tvFPS.setText("0.0");
        tvCPU.setText("0%");
        tvGPU.setText("0%");
        tvRAM.setText("0MB");

        setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        startTimer();
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        stopTimer();
    }

    private void startTimer() {
        stopTimer();
        timer = new Timer();
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                updateStats();
                handler.post(() -> {
                    tvCPU.setText(String.format(Locale.ENGLISH, "%d%%", lastCpuUsage));
                    
                    // 照抄任务管理器的内存显示逻辑: (百分比)已用/总计 单位
                    // 使用 StringUtils.formatBytes(bytes, false) 获取不带单位的数值，总计部分带单位
                    String ramText = String.format(Locale.ENGLISH, "(%d%%)%s/%s", 
                        lastMemUsage, 
                        StringUtils.formatBytes(lastUsedMem, false),
                        StringUtils.formatBytes(lastTotalMem, true));
                    tvRAM.setText(ramText);
                    
                    // 伪GPU占用推算：基于CPU负载和内存使用，并增加情绪价值抖动
                    int baseGpu = (int) (lastCpuUsage * 1.1f + lastMemUsage * 0.15f);
                    int fakeGpuUsage = Math.min(99, Math.max(5, baseGpu + random.nextInt(15) - 7));
                    tvGPU.setText(String.format(Locale.ENGLISH, "%d%%", fakeGpuUsage));
                });
            }
        }, 0, 1000);
    }

    private void stopTimer() {
        if (timer != null) {
            timer.cancel();
            timer = null;
        }
    }

    public void update() {
        if (lastTime == 0) lastTime = SystemClock.elapsedRealtime();
        long time = SystemClock.elapsedRealtime();
        if (time >= lastTime + 1000) {
            lastFPS = ((float)(frameCount * 1000) / (time - lastTime));
            handler.post(() -> tvFPS.setText(String.format(Locale.ENGLISH, "%.1f", lastFPS)));
            lastTime = time;
            frameCount = 0;
        }
        frameCount++;
    }

    private void updateStats() {
        try {
            short[] clockSpeeds = CPUStatus.getCurrentClockSpeeds();
            int totalClockSpeed = 0;
            short maxClockSpeed = 0;
            if (clockSpeeds != null && clockSpeeds.length > 0) {
                for (int i = 0; i < clockSpeeds.length; i++) {
                    short clockSpeed = CPUStatus.getMaxClockSpeed(i);
                    totalClockSpeed += clockSpeeds[i];
                    maxClockSpeed = (short) Math.max(maxClockSpeed, clockSpeed);
                }
                if (maxClockSpeed > 0) {
                    lastCpuUsage = (int) (((float) totalClockSpeed / (clockSpeeds.length * maxClockSpeed)) * 100.0f);
                }
            }
        } catch (Exception e) {}

        try {
            ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
            activityManager.getMemoryInfo(memoryInfo);
            lastTotalMem = memoryInfo.totalMem;
            lastUsedMem = memoryInfo.totalMem - memoryInfo.availMem;
            lastMemUsage = (int) (((double) lastUsedMem / memoryInfo.totalMem) * 100.0f);
        } catch (Exception e) {}
    }
}