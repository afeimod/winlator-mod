package com.winlator.core;

import com.winlator.container.GraphicsDrivers;

public abstract class DefaultVersion {
    public static final String BOX64 = "0.3.6";
    public static final String TURNIP = "25.1.0";
    public static final String VORTEK = "2.1";
    public static final String ZINK = "22.2.5";
    public static final String VIRGL = "23.1.9";
    public static final String D8VK = "1.0";
    public static final String VKD3D = "2.13";
    public static final String WINED3D = WineInfo.MAIN_WINE_VERSION;
    public static final String CNC_DDRAW = "6.6";
    public static final String SOUNDFONT = "SONiVOX-EAS-GM-Wavetable";
    public static final String MINOR_DXVK = "1.10.3";
    public static final String MAJOR_DXVK = "2.4.1";

    public static String DXVK() {
        return DXVK(null);
    }

    public static String DXVK(String graphicsDriver) {
        int vkApiVersion = 0;
        if (graphicsDriver != null && graphicsDriver.equals(GraphicsDrivers.VORTEK)) vkApiVersion = GPUHelper.vkGetApiVersion();
        return graphicsDriver == null || graphicsDriver.equals(GraphicsDrivers.TURNIP) || vkApiVersion >= GPUHelper.vkMakeVersion(1, 3, 0) ? MAJOR_DXVK : MINOR_DXVK;
    }
}