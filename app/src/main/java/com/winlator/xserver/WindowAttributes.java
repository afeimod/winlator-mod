package com.winlator.xserver;

import com.winlator.core.Bitmask;
import com.winlator.xconnector.XInputStream;

public class WindowAttributes {
    public static final int FLAG_BACKGROUND_PIXMAP = 1<<0;
    public static final int FLAG_BACKGROUND_PIXEL = 1<<1;
    public static final int FLAG_BORDER_PIXMAP = 1<<2;
    public static final int FLAG_BORDER_PIXEL = 1<<3;
    public static final int FLAG_BIT_GRAVITY = 1<<4;
    public static final int FLAG_WIN_GRAVITY = 1<<5;
    public static final int FLAG_BACKING_STORE = 1<<6;
    public static final int FLAG_BACKING_PLANES = 1<<7;
    public static final int FLAG_BACKING_PIXEL = 1<<8;
    public static final int FLAG_OVERRIDE_REDIRECT = 1<<9;
    public static final int FLAG_SAVE_UNDER = 1<<10;
    public static final int FLAG_EVENT_MASK = 1<<11;
    public static final int FLAG_DO_NOT_PROPAGATE_MASK = 1<<12;
    public static final int FLAG_COLORMAP = 1<<13;
    public static final int FLAG_CURSOR = 1<<14;
    public static final int FLAG_MAPPED = 1<<15;
    public static final int FLAG_ENABLED = 1<<16;
    public static final int FLAG_TRANSPARENT = 1<<17;
    public static final int FLAG_RENDER_SUBWINDOWS = 1<<18;
    public static final int FLAG_VIEWABLE = 1<<19;
    public enum BackingStore {NOT_USEFUL, WHEN_MAPPED, ALWAYS}
    public enum WindowClass {COPY_FROM_PARENT, INPUT_OUTPUT, INPUT_ONLY}
    public enum BitGravity {FORGET, NORTH_WEST, NORTH, NORTH_EAST, WEST, CENTER, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, STATIC}
    public enum WinGravity {UNMAP, NORTH_WEST, NORTH, NORTH_EAST, WEST, CENTER, EAST, SOUTH_WEST, SOUTH, SOUTH_EAST, STATIC}
    private int backingPixel = 0;
    private int backingPlanes = 1;
    private BackingStore backingStore = BackingStore.NOT_USEFUL;
    private BitGravity bitGravity = BitGravity.CENTER;
    private Cursor cursor;
    private Bitmask doNotPropagateMask = new Bitmask(0);
    private Bitmask eventMask = new Bitmask(0);
    private WinGravity winGravity = WinGravity.CENTER;
    private WindowClass windowClass = WindowClass.INPUT_OUTPUT;
    private final Bitmask attributeFlags = new Bitmask(new int[]{FLAG_ENABLED, FLAG_RENDER_SUBWINDOWS, FLAG_VIEWABLE});
    public final Window window;

    public WindowAttributes(Window window) {
        this.window = window;
    }

    public int getBackingPixel() {
        return backingPixel;
    }

    public int getBackingPlanes() {
        return backingPlanes;
    }

    public BackingStore getBackingStore() {
        return backingStore;
    }

    public BitGravity getBitGravity() {
        return bitGravity;
    }

    public Cursor getCursor() {
        Window parent = window.getParent();
        return cursor == null && parent != null ? parent.attributes.getCursor() : cursor;
    }

    public Bitmask getEventMask() {
        return eventMask;
    }

    public Bitmask getDoNotPropagateMask() {
        return doNotPropagateMask;
    }

    public boolean isMapped() {
        return attributeFlags.isSet(FLAG_MAPPED);
    }

    public void setMapped(boolean mapped) {
        attributeFlags.set(FLAG_MAPPED, mapped);
    }

    public boolean isOverrideRedirect() {
        return attributeFlags.isSet(FLAG_OVERRIDE_REDIRECT);
    }

    public boolean isSaveUnder() {
        return attributeFlags.isSet(FLAG_SAVE_UNDER);
    }

    public WinGravity getWinGravity() {
        return winGravity;
    }

    public WindowClass getWindowClass() {
        return windowClass;
    }

    public void setWindowClass(WindowClass windowClass) {
        this.windowClass = windowClass;
    }

    public Window getWindow() {
        return window;
    }

    public boolean isEnabled() {
        return attributeFlags.isSet(FLAG_ENABLED);
    }

    public void setEnabled(boolean enabled) {
        attributeFlags.set(FLAG_ENABLED, enabled);
    }

    public boolean isRenderSubwindows() {
        return attributeFlags.isSet(FLAG_RENDER_SUBWINDOWS);
    }

    public void setRenderSubwindows(boolean renderSubwindows) {
        attributeFlags.set(FLAG_RENDER_SUBWINDOWS, renderSubwindows);
    }

    public boolean isViewable() {
        return attributeFlags.isSet(FLAG_VIEWABLE);
    }

    public void setViewable(boolean viewable) {
        attributeFlags.set(FLAG_VIEWABLE, viewable);
    }

    public void update(Bitmask valueMask, XInputStream inputStream, XClient client) {
        for (int index : valueMask) {
            switch (index) {
                case FLAG_BACKGROUND_PIXEL:
                    window.getContent().fillColor(inputStream.readInt());
                    break;
                case FLAG_BACKING_PIXEL:
                    backingPixel = inputStream.readInt();
                    break;
                case FLAG_BACKING_PLANES:
                    backingPlanes = inputStream.readInt();
                    break;
                case FLAG_BIT_GRAVITY:
                    bitGravity = BitGravity.values()[inputStream.readInt()];
                    break;
                case FLAG_WIN_GRAVITY:
                    winGravity = WinGravity.values()[inputStream.readInt()];
                    break;
                case FLAG_BACKING_STORE:
                    backingStore = BackingStore.values()[inputStream.readInt()];
                    break;
                case FLAG_SAVE_UNDER:
                case FLAG_OVERRIDE_REDIRECT:
                    attributeFlags.set(index, inputStream.readInt() == 1);
                    break;
                case FLAG_EVENT_MASK:
                    eventMask = new Bitmask(inputStream.readInt());
                    break;
                case FLAG_DO_NOT_PROPAGATE_MASK:
                    doNotPropagateMask = new Bitmask(inputStream.readInt());
                    break;
                case FLAG_CURSOR:
                    cursor = client.xServer.cursorManager.getCursor(inputStream.readInt());
                    break;
                case FLAG_BACKGROUND_PIXMAP:
                case FLAG_BORDER_PIXMAP:
                case FLAG_BORDER_PIXEL:
                case FLAG_COLORMAP:
                    inputStream.skip(4);
                    break;
            }
        }

        client.xServer.windowManager.triggerOnUpdateWindowAttributes(window, valueMask);
    }

    public boolean isTransparent() {
        return attributeFlags.isSet(FLAG_TRANSPARENT);
    }

    public void setTransparent(boolean transparent) {
        attributeFlags.set(FLAG_TRANSPARENT, transparent);
    }
}