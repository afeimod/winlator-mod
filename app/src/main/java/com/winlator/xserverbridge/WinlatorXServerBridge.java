package com.winlator.xserverbridge;

import com.winlator.winhandler.MouseEventFlags;
import com.winlator.winhandler.WinHandler;
import com.winlator.xserver.Pointer;
import com.winlator.xserver.XKeycode;
import com.winlator.xserver.XServer;

public class WinlatorXServerBridge implements IXServerBridge {
    private final XServer xServer;
    // idx 对应其 keycode
    private final XKeycode[] xKeycodesByCode = new XKeycode[128];

    public WinlatorXServerBridge(XServer xServer) {
        this.xServer = xServer;
        for (XKeycode xKeycode : XKeycode.values()) {
            xKeycodesByCode[xKeycode.id] = xKeycode;
        }
    }

    @Override
    public WinHandler getWinHandler() {
        return xServer.getWinHandler();
    }

    @Override
    public int getScreenWidth() {
        return xServer.screenInfo.width;
    }

    @Override
    public int getScreenHeight() {
        return xServer.screenInfo.height;
    }

    @Override
    public boolean isStretchFullscreen() {
        return xServer.getRenderer().isFullscreen();
    }

    @Override
    public int getPointerX() {
        return xServer.pointer.getX();
    }

    @Override
    public int getPointerY() {
        return xServer.pointer.getY();
    }

    @Override
    public void injectPointerMove(int x, int y) {
        xServer.injectPointerMove(x, y);
    }

    @Override
    public void injectPointerMoveDelta(int dx, int dy) {
        if (xServer.isRelativeMouseMovement()) {
            xServer.getWinHandler().mouseEvent(MouseEventFlags.MOVE, dx, dy, 0);
        } else {
            xServer.injectPointerMoveDelta(dx, dy);
        }
    }

    @Override
    public void injectPointerButtonPress(int btnCode) {
        xServer.injectPointerButtonPress(pointerBtnCode2Enum(btnCode));
    }

    @Override
    public void injectPointerButtonRelease(int btnCode) {
        xServer.injectPointerButtonRelease(pointerBtnCode2Enum(btnCode));
    }

    @Override
    public boolean isPointerButtonPressed(int btnCode) {
        return xServer.pointer.isButtonPressed(pointerBtnCode2Enum(btnCode));
    }

    private Pointer.Button pointerBtnCode2Enum(int btnCode) {
        return Pointer.Button.values()[btnCode - 1];
    }

    @Override
    public void injectKeyPress(byte keycode, int keysym) {
        xServer.injectKeyPress(xKeycodesByCode[keycode], keysym);
    }

    @Override
    public void injectKeyRelease(byte keycode) {
        xServer.injectKeyRelease(xKeycodesByCode[keycode]);
    }
}
