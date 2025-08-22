package com.winlator.core;

import android.app.Activity;
import android.content.Context;
import android.os.BatteryManager;

public abstract class BatteryUtils {
    public static int getCapacity(Context context) {
        final String POWER_PROFILE_CLASS = "com.android.internal.os.PowerProfile";

        try {
            Object powerProfile = Class.forName(POWER_PROFILE_CLASS)
                .getConstructor(Context.class)
                .newInstance(context);

            Double capacity = ((Double)Class
                .forName(POWER_PROFILE_CLASS)
                .getMethod("getBatteryCapacity")
                .invoke(powerProfile));
            return capacity != null ? capacity.intValue() : 0;
        }
        catch (Exception e) {
            return 0;
        }
    }

    public static int getCurrentMicroamperes(Activity activity) {
        BatteryManager batteryManager = (BatteryManager)activity.getSystemService(Context.BATTERY_SERVICE);
        int currentMicroamperes = batteryManager.getIntProperty(BatteryManager.BATTERY_PROPERTY_CURRENT_NOW);
        currentMicroamperes = currentMicroamperes != 0 && currentMicroamperes != Integer.MIN_VALUE ? Math.abs(currentMicroamperes) : 0;
        if (currentMicroamperes <= 1000) currentMicroamperes *= 1000;
        return currentMicroamperes;
    }

    public static float computePower(int currentMicroamperes, float voltage) {
        return (currentMicroamperes / 1000000.0f) * voltage;
    }
}
