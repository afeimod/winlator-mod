package com.winlator.core;

import com.winlator.XServerDisplayActivity;
import com.winlator.container.Container;
import com.winlator.winhandler.WinHandler;
import com.winlator.xserver.ScreenInfo;
import com.winlator.xserver.Window;

import java.util.Locale;

public class Win32AppWorkarounds {
    private final short taskAffinityMask;
    private final short taskAffinityMaskWoW64;
    private final XServerDisplayActivity activity;

    private interface Workaround {}

    private static class MultiWorkaround implements Workaround {
        private final Workaround[] list;

        public MultiWorkaround(Workaround... list) {
            this.list = list;
        }
    }

    private interface WindowWorkaround extends Workaround {
        void apply(Window window);
    }

    private interface EnvVarsWorkaround extends Workaround {
        void apply(EnvVars envVars);
    }

    private interface ScreenSizeWorkaround extends Workaround {
        String getValue();
    }

    private interface GraphicsDriverWorkaround extends Workaround {
        String getValue();
    }

    private interface DXWrapperWorkaround extends Workaround {
        String getValue();
    }

    private interface DXWrapperConfigWorkaround extends Workaround {
        void setValue(String dxwrapper, KeyValueSet config);
    }

    private interface WinComponentsWorkaround extends Workaround {
        void setValue(KeyValueSet wincomponents);
    }

    public Win32AppWorkarounds(XServerDisplayActivity activity) {
        this.activity = activity;
        Container container = activity.getContainer();
        taskAffinityMask = (short)ProcessHelper.getAffinityMask(container.getCPUList(true));
        taskAffinityMaskWoW64 = (short)ProcessHelper.getAffinityMask(container.getCPUListWoW64(true));
    }

    private void applyWorkaround(Workaround workaround) {
        if (workaround instanceof EnvVarsWorkaround) {
            ((EnvVarsWorkaround)workaround).apply(activity.getOverrideEnvVars());
        }
        else if (workaround instanceof ScreenSizeWorkaround) {
            activity.setScreenInfo(new ScreenInfo(((ScreenSizeWorkaround)workaround).getValue()));
        }
        else if (workaround instanceof DXWrapperWorkaround) {
            activity.setDXWrapper(((DXWrapperWorkaround)workaround).getValue());
        }
        else if (workaround instanceof DXWrapperConfigWorkaround) {
            ((DXWrapperConfigWorkaround)workaround).setValue(activity.getDXWrapper(), activity.getDXWrapperConfig());
        }
        else if (workaround instanceof GraphicsDriverWorkaround) {
            activity.setGraphicsDriver(((GraphicsDriverWorkaround)workaround).getValue());
        }
        else if (workaround instanceof WinComponentsWorkaround) {
            KeyValueSet wincomponents = new KeyValueSet(Container.DEFAULT_WINCOMPONENTS);
            ((WinComponentsWorkaround)workaround).setValue(wincomponents);
            activity.setWinComponents(wincomponents.toString());
        }
    }

    public void applyStartupWorkarounds(String className) {
        Workaround workaround = getWorkaroundFor(className);
        if (workaround == null) return;

        if (workaround instanceof MultiWorkaround) {
            for (Workaround workaround2 : ((MultiWorkaround)workaround).list) applyWorkaround(workaround2);
        }
        else applyWorkaround(workaround);
    }

    private void setProcessAffinity(Window window, int processAffinity) {
        int processId = window.getProcessId();
        String className = window.getClassName();
        WinHandler winHandler = activity.getWinHandler();

        if (className.equals("steam.exe")) return;

        if (processId > 0) {
            winHandler.setProcessAffinity(processId, processAffinity);
        }
        else if (!className.isEmpty()) {
            winHandler.setProcessAffinity(window.getClassName(), processAffinity);
        }
    }

    public void applyWindowWorkarounds(Window window) {
        Workaround workaround = getWorkaroundFor(window.getClassName());
        if (workaround instanceof WindowWorkaround) {
            ((WindowWorkaround)workaround).apply(window);
        }
        else if (workaround instanceof MultiWorkaround) {
            for (Workaround workaround2 : ((MultiWorkaround) workaround).list) {
                if (workaround2 instanceof WindowWorkaround) {
                    ((WindowWorkaround)workaround2).apply(window);
                    break;
                }
            }
        }

        int windowGroup = window.getWMHintsValue(Window.WMHints.WINDOW_GROUP);
        boolean canApplyProcessAffinity = window.isRenderable() && !window.getClassName().isEmpty() && windowGroup == window.id;
        if (canApplyProcessAffinity) {
            int processAffinity = window.isWoW64() ? taskAffinityMaskWoW64 : taskAffinityMask;
            if (processAffinity != 0) setProcessAffinity(window, processAffinity);
        }
    }

    private Workaround getWorkaroundFor(String className) {
        String appIdentifier;
        if (className.startsWith("steam://")) {
            appIdentifier = className.substring(className.lastIndexOf("/") + 1);
        }
        else appIdentifier = className.toLowerCase(Locale.ENGLISH);

        switch (appIdentifier) {
            case "sonicgenerations.exe":
            case "71340":
            case "valkyria.exe":
            case "294860":
                return (EnvVarsWorkaround) (envVars) -> envVars.put("WINEESYNC", "0");
            case "blacklist_game.exe":
            case "blacklist_dx11_game.exe":
                return (EnvVarsWorkaround) (envVars) -> envVars.put("WINEOVERRIDEAFFINITYMASK", taskAffinityMaskWoW64);
            case "fate.exe":
                return (ScreenSizeWorkaround) () -> "1024x768";
            case "ffxii_tza.exe":
                ScreenInfo screenInfo = activity.getScreenInfo();
                return (ScreenSizeWorkaround) () -> (screenInfo.width+4)+"x"+(screenInfo.height+4);
            case "chronocross_launcher.exe":
                return (WindowWorkaround) (window) -> window.attributes.setTransparent(true);
            default:
                return null;
        }
    }
}