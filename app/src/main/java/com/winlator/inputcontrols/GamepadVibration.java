package com.winlator.inputcontrols;

import android.content.Context;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.view.InputDevice;

import com.winlator.math.Mathf;

public class GamepadVibration {
    private static final int MAX_DURATION_MILLIS = 1000 * 60;
    private final Vibrator[] vibrators = new Vibrator[2];
    private final short[] currentSpeed = new short[2];
    private final boolean[] vibrating = new boolean[2];

    protected GamepadVibration(Context context) {
        VibratorManager manager = null;
        Vibrator vibrator = null;

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.S) {
            manager = (VibratorManager)context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
        }
        else vibrator = (Vibrator)context.getSystemService(Context.VIBRATOR_SERVICE);

        vibrators[0] = getVibratorAt(0, manager, vibrator);
        vibrators[1] = getVibratorAt(1, manager, vibrator);
    }

    protected GamepadVibration(String id) {
        VibratorManager manager = null;
        Vibrator vibrator = null;

        for (int deviceId : InputDevice.getDeviceIds()) {
            InputDevice device = InputDevice.getDevice(deviceId);
            if (device != null && device.getDescriptor().equals(id)) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                    manager = device.getVibratorManager();
                }
                else vibrator = device.getVibrator();
                break;
            }
        }

        vibrators[0] = getVibratorAt(0, manager, vibrator);
        vibrators[1] = getVibratorAt(1, manager, vibrator);
    }

    private static Vibrator getVibratorAt(int index, VibratorManager manager, Vibrator defaultVibrator) {
        Vibrator result;
        if (manager != null && Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            int[] ids = manager.getVibratorIds();
            result = ids.length >= 2 && index < ids.length ? manager.getVibrator(index) : manager.getDefaultVibrator();
        }
        else result = defaultVibrator;
        return result != null && result.hasVibrator() ? result : null;
    }

    private void vibrateAt(int index, short newSpeed) {
        if (newSpeed == 0) {
            if (vibrating[index]) vibrators[index].cancel();
            vibrating[index] = false;
        }
        else if (newSpeed != currentSpeed[index]) {
            long[] timings = new long[]{0, MAX_DURATION_MILLIS};
            int[] amplitudes = {0, newSpeed};
            vibrators[index].vibrate(VibrationEffect.createWaveform(timings, amplitudes, -1));
            vibrating[index] = true;
        }

        currentSpeed[index] = newSpeed;
    }

    private short parseAmplitude(int motorSpeed) {
        return (short)Mathf.roundTo((motorSpeed / 65535.0f) * 255, 15);
    }

    public void cancel() {
        for (byte i = 0; i < vibrators.length; i++) {
            if (vibrating[i]) {
                currentSpeed[i] = 0;
                vibrators[i].cancel();
                vibrating[i] = false;
            }
        }
    }

    public void vibrate(int leftMotorSpeed, int rightMotorSpeed) {
        if (vibrators[0] != null && vibrators[1] != null) {
            short speedX = parseAmplitude(leftMotorSpeed);
            short speedY = parseAmplitude(rightMotorSpeed);

            if (vibrators[0] == vibrators[1]) {
                short avgSpeed = (short)((speedX + speedY) * 0.5f);
                vibrateAt(0, avgSpeed);
            }
            else {
                vibrateAt(0, speedX);
                vibrateAt(1, speedY);
            }
        }
    }
}
