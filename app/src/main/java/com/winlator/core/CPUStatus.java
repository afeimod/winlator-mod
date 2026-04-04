package com.winlator.core;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Locale;

public abstract class CPUStatus {
    public static short[] getCurrentClockSpeeds() {
        int numProcessors = Runtime.getRuntime().availableProcessors();
        short[] clockSpeeds = new short[numProcessors];
        for (int i = 0; i < numProcessors; i++) {
            int currFreq = FileUtils.readInt("/sys/devices/system/cpu/cpu"+i+"/cpufreq/scaling_cur_freq");
            clockSpeeds[i] = (short)(currFreq / 1000);
        }
        return clockSpeeds;
    }

    public static short getMaxClockSpeed(int cpuIndex) {
        int maxFreq = FileUtils.readInt("/sys/devices/system/cpu/cpu"+cpuIndex+"/cpufreq/cpuinfo_max_freq");
        return (short)(maxFreq / 1000);
    }

    public static int getTemperature() {
        final String[] sensorFiles = {
            "/sys/devices/system/cpu/cpu0/cpufreq/cpu_temp",
            "/sys/devices/system/cpu/cpu0/cpufreq/FakeShmoo_cpu_temp",
            "/sys/devices/platform/tegra-i2c.3/i2c-4/4-004c/temperature",
            "/sys/devices/platform/omap/omap_temp_sensor.0/temperature",
            "/sys/devices/platform/tegra_tmon/temp1_input",
            "/sys/devices/platform/s5p-tmu/temperature",
            "/sys/devices/platform/s5p-tmu/curr_temp",
            "/sys/devices/virtual/thermal/thermal_zone10/temp",
            "/sys/devices/virtual/thermal/thermal_zone1/temp",
            "/sys/devices/virtual/thermal/thermal_zone0/temp",
            "/sys/class/thermal/thermal_zone0/temp",
            "/sys/class/thermal/thermal_zone1/temp",
            "/sys/class/thermal/thermal_zone3/temp",
            "/sys/class/thermal/thermal_zone4/temp",
            "/sys/class/hwmon/hwmon0/device/temp1_input",
            "/sys/class/hwmon/hwmonX/temp1_input",
            "/sys/class/i2c-adapter/i2c-4/4-004c/temperature",
            "/sys/kernel/debug/tegra_thermal/temp_tj",
            "/sys/htc/cpu_temp",
            "/sys/devices/platform/tegra-i2c.3/i2c-4/4-004c/ext_temperature",
            "/sys/devices/platform/tegra-tsensor/tsensor_temperature",
            "/sys/devices/virtual/sec/sec-lp-thermistor/temperature"
        };

        for (String path : sensorFiles) {
            File file = new File(path);
            if (file.exists()) {
                try (RandomAccessFile reader = new RandomAccessFile(file , "r")) {
                    String value = reader.readLine();
                    int temperature = value != null ? Integer.parseInt(value) : 0;
                    if (temperature > 100 && temperature <= 1000) {
                        return (int)(temperature / 10.0f);
                    }
                    else if (temperature > 1000 && temperature <= 10000) {
                        return (int)(temperature / 100.0f);
                    }
                    else if (temperature > 10000) {
                        return (int)(temperature / 1000.0f);
                    }
                }
                catch (IOException | NumberFormatException e) {}
            }
        }

        return 0;
    }

    public static String formatClockSpeed(int clockSpeed) {
        return clockSpeed >= 1000 ? String.format(Locale.ENGLISH, "%.2f", clockSpeed / 1000.0f)+" GHz" : clockSpeed+" MHz";
    }
}