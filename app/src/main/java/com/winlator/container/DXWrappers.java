package com.winlator.container;

import com.winlator.core.KeyValueSet;

public abstract class DXWrappers {
    public static final String WINED3D = "wined3d";
    public static final String DXVK = "dxvk";
    public static final String VKD3D = "vkd3d";
    public static final String CNC_DDRAW = "cnc-ddraw";

    public static String getName(String identifier) {
        switch (identifier) {
            case WINED3D: return "WineD3D";
            case DXVK: return "DXVK";
            case VKD3D: return "VKD3D";
            case CNC_DDRAW: return "CNC DDraw";
            default: return "None";
        }
    }

    public static String parseIdentifier(String dxwrapper) {
        if (dxwrapper == null || dxwrapper.isEmpty()) return Container.DEFAULT_DXWRAPPER;
        return dxwrapper.equals(WINED3D) || dxwrapper.equals(DXVK) ? dxwrapper : Container.DEFAULT_DXWRAPPER;
    }

    public static KeyValueSet[] parseConfigs(String dxwrapper, String dxwrapperConfig) {
        if (dxwrapperConfig == null || dxwrapperConfig.isEmpty()) return new KeyValueSet[]{new KeyValueSet(), new KeyValueSet()};
        int separator = dxwrapperConfig.indexOf("|");
        if (separator != -1) {
            String first = dxwrapperConfig.substring(0, separator);
            String second = dxwrapperConfig.substring(separator + 1);
            return new KeyValueSet[]{new KeyValueSet(first), new KeyValueSet(second)};
        }
        else if (dxwrapper != null) {
            if (dxwrapper.equals(WINED3D) || dxwrapper.equals(DXVK)) {
                return new KeyValueSet[]{new KeyValueSet(dxwrapperConfig), new KeyValueSet()};
            }
            else if (dxwrapper.equals(VKD3D)) {
                return new KeyValueSet[]{new KeyValueSet(), new KeyValueSet(dxwrapperConfig)};
            }
        }
        return new KeyValueSet[]{new KeyValueSet(), new KeyValueSet()};
    }
}
