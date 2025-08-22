package com.winlator.renderer;

import com.winlator.xserver.ScreenInfo;
import com.winlator.xserver.Window;

public class FullscreenTransformation {
    public short x;
    public short y;
    public short width;
    public short height;
    private final Window window;

    public FullscreenTransformation(Window window) {
        this.window = window;
    }

    public void update(ScreenInfo screenInfo, short originWidth, short originHeight) {
        short targetHeight = (short)Math.min(screenInfo.height, ((float)screenInfo.width / originWidth) * originHeight);
        short targetWidth = (short)(((float)targetHeight / originHeight) * originWidth);
        this.x = (short)((screenInfo.width - targetWidth) * 0.5f);
        this.y = (short)((screenInfo.height - targetHeight) * 0.5f);
        this.width = targetWidth;
        this.height = targetHeight;
    }

    public short[] transformPointerCoords(short x, short y) {
        short[] localPoint = window.rootPointToLocal(x, y, true);
        float scaleX = (float)window.getWidth() / this.width;
        float scaleY = (float)window.getHeight() / this.height;
        short transformedX = (short)Math.max(0, localPoint[0] * scaleX + window.getRootX());
        short transformedY = (short)Math.max(0, localPoint[1] * scaleY + window.getRootY());
        return new short[]{transformedX, transformedY};
    }
}
