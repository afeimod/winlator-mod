package com.winlator.winhandler;

import com.winlator.core.KeyValueSet;

public class GamepadPlayerConfig {
    public static final byte MODE_EXTERNAL_CONTROLLER = 0;
    public static final byte MODE_CONTROLS_PROFILE = 1;
    public final byte mode;
    public final String name;
    public final boolean vibration;

    public GamepadPlayerConfig(String values) {
        KeyValueSet config = new KeyValueSet(values);
        mode = (byte)config.getInt("mode");
        name = config.get("name");
        vibration = config.getBoolean("vibration");
    }
}
