package com.winlator.xserver;

import java.util.ArrayList;
import java.util.Arrays;

public abstract class Atom {
    public static final byte PRIMARY = 1;
    public static final byte SECONDARY = 2;
    public static final byte ARC = 3;
    public static final byte ATOM = 4;
    public static final byte BITMAP = 5;
    public static final byte CARDINAL = 6;
    public static final byte COLORMAP = 7;
    public static final byte CURSOR = 8;
    public static final byte CUT_BUFFER0 = 9;
    public static final byte CUT_BUFFER1 = 10;
    public static final byte CUT_BUFFER2 = 11;
    public static final byte CUT_BUFFER3 = 12;
    public static final byte CUT_BUFFER4 = 13;
    public static final byte CUT_BUFFER5 = 14;
    public static final byte CUT_BUFFER6 = 15;
    public static final byte CUT_BUFFER7 = 16;
    public static final byte DRAWABLE = 17;
    public static final byte FONT = 18;
    public static final byte INTEGER = 19;
    public static final byte PIXMAP = 20;
    public static final byte POINT = 21;
    public static final byte RECTANGLE = 22;
    public static final byte RESOURCE_MANAGER = 23;
    public static final byte RGB_COLOR_MAP = 24;
    public static final byte RGB_BEST_MAP = 25;
    public static final byte RGB_BLUE_MAP = 26;
    public static final byte RGB_DEFAULT_MAP = 27;
    public static final byte RGB_GRAY_MAP = 28;
    public static final byte RGB_GREEN_MAP = 29;
    public static final byte RGB_RED_MAP = 30;
    public static final byte STRING = 31;
    public static final byte VISUALID = 32;
    public static final byte WINDOW = 33;
    public static final byte WM_COMMAND = 34;
    public static final byte WM_HINTS = 35;
    public static final byte WM_CLIENT_MACHINE = 36;
    public static final byte WM_ICON_NAME = 37;
    public static final byte WM_ICON_SIZE = 38;
    public static final byte WM_NAME = 39;
    public static final byte WM_NORMAL_HINTS = 40;
    public static final byte WM_SIZE_HINTS = 41;
    public static final byte WM_ZOOM_HINTS = 42;
    public static final byte MIN_SPACE = 43;
    public static final byte NORM_SPACE = 44;
    public static final byte MAX_SPACE = 45;
    public static final byte END_SPACE = 46;
    public static final byte SUPERSC_LPT_X = 47;
    public static final byte SUPERSC_LPT_Y = 48;
    public static final byte SUBSC_LPT_X = 49;
    public static final byte SUBSC_LPT_Y = 50;
    public static final byte UNDERLINE_POSITION = 51;
    public static final byte UNDERLINE_THICKNESS = 52;
    public static final byte STRIKEOUT_ASCENT = 53;
    public static final byte STRIKEOUT_DESCENT = 54;
    public static final byte ITALIC_ANGLE = 55;
    public static final byte X_HEIGHT = 56;
    public static final byte QUAD_WIDTH = 57;
    public static final byte WEIGHT = 58;
    public static final byte POINT_SIZE = 59;
    public static final byte RESOLUTION = 60;
    public static final byte COPYRIGHT = 61;
    public static final byte NOTICE = 62;
    public static final byte FONT_NAME = 63;
    public static final byte FAMILY_NAME = 64;
    public static final byte FULL_NAME = 65;
    public static final byte CAP_HEIGHT = 66;
    public static final byte WM_CLASS = 67;
    public static final byte WM_TRANSIENT_FOR = 68;
    public static final byte _MOTIF_WM_HINTS = 69;
    public static final byte _NET_WM_PID = 70;
    public static final byte _NET_WM_WINDOW_TYPE = 71;
    public static final byte _NET_WM_HWND = 72;
    public static final byte _NET_WM_WOW64 = 73;
    public static final byte _NET_WM_SURFACE = 74;

    private static final ArrayList<String> atoms = new ArrayList<>(Arrays.asList(null, "PRIMARY", "SECONDARY", "ARC", "ATOM", "BITMAP", "CARDINAL", "COLORMAP", "CURSOR", "CUT_BUFFER0", "CUT_BUFFER1", "CUT_BUFFER2", "CUT_BUFFER3", "CUT_BUFFER4", "CUT_BUFFER5", "CUT_BUFFER6", "CUT_BUFFER7", "DRAWABLE", "FONT", "INTEGER", "PIXMAP", "POINT", "RECTANGLE", "RESOURCE_MANAGER", "RGB_COLOR_MAP", "RGB_BEST_MAP", "RGB_BLUE_MAP", "RGB_DEFAULT_MAP", "RGB_GRAY_MAP", "RGB_GREEN_MAP", "RGB_RED_MAP", "STRING", "VISUALID", "WINDOW", "WM_COMMAND", "WM_HINTS", "WM_CLIENT_MACHINE", "WM_ICON_NAME", "WM_ICON_SIZE", "WM_NAME", "WM_NORMAL_HINTS", "WM_SIZE_HINTS", "WM_ZOOM_HINTS", "MIN_SPACE", "NORM_SPACE", "MAX_SPACE", "END_SPACE", "SUPERSC.LPT_X", "SUPERSC.LPT_Y", "SUBSC.LPT_X", "SUBSC.LPT_Y", "UNDERLINE_POSITION", "UNDERLINE_THICKNESS", "STRIKEOUT_ASCENT", "STRIKEOUT_DESCENT", "ITALIC_ANGLE", "X_HEIGHT", "QUAD_WIDTH", "WEIGHT", "POINT_SIZE", "RESOLUTION", "COPYRIGHT", "NOTICE", "FONT_NAME", "FAMILY_NAME", "FULL_NAME", "CAP_HEIGHT", "WM_CLASS", "WM_TRANSIENT_FOR", "_MOTIF_WM_HINTS", "_NET_WM_PID", "_NET_WM_WINDOW_TYPE", "_NET_WM_HWND", "_NET_WM_WOW64", "_NET_WM_SURFACE"));

    public synchronized static String getName(int id) {
        return atoms.get(id);
    }

    public synchronized static int getId(String name) {
        if (name == null) return 0;
        for (int i = 0; i < atoms.size(); i++) if (name.equals(atoms.get(i))) return i;
        return -1;
    }

    public synchronized static int internAtom(String name) {
        int id = getId(name);
        if (id == -1) {
            id = atoms.size();
            atoms.add(name);
        }
        return id;
    }

    public synchronized static boolean isValid(int id) {
        return id > 0 && id < atoms.size();
    }
}
