package com.winlator.xserverbridge;

import com.termux.x11.LorieView;
import com.winlator.winhandler.MouseEventFlags;
import com.winlator.winhandler.WinHandler;
import com.winlator.xserver.Pointer;
import com.winlator.xserver.XServer;

public class TX11XServerBridge implements IXServerBridge {
    private final LorieView lorieView;
    private final XServer xServer;

    public TX11XServerBridge(LorieView lorieView, XServer xServer) {
        this.lorieView = lorieView;
        this.xServer = xServer;
    }

    @Override
    public WinHandler getWinHandler() {
        return xServer.getWinHandler();
    }

    @Override
    public int getScreenWidth() {
        return lorieView.p.x;
    }

    @Override
    public int getScreenHeight() {
        return lorieView.p.y;
    }

    @Override
    public boolean isStretchFullscreen() {
        return lorieView.isStretchFullscreen();
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
        lorieView.sendMouseEvent(x, y, 0, false, false);
    }

    @Override
    public void injectPointerMoveDelta(int dx, int dy) {
        if (xServer.isRelativeMouseMovement()) {
            // 游戏模式：直接走 Windows 相对移动，彻底解决划屏“方形框”束缚
            xServer.getWinHandler().mouseEvent(MouseEventFlags.MOVE, dx, dy, 0);
        } else {
            // 桌面模式：直接走 TX11 相对移动，保证鼠标移动丝滑、不乱动
            lorieView.sendMouseEvent(dx, dy, 0, false, true);
            // 同步内部坐标仅用于 UI 参考
            xServer.injectPointerMoveDelta(dx, dy);
        }
    }

    @Override
    public void injectPointerButtonPress(int btnCode) {
        Pointer.Button button = pointerBtnCode2Enum(btnCode);
        xServer.injectPointerButtonPress(button);

        if (btnCode == 4 || btnCode == 5) {
            lorieView.sendMouseEvent(0, btnCode == 4 ? -60 : 60, 4, false, true);
        } else {
            if (xServer.isRelativeMouseMovement()) {
                // 游戏模式：按键直接走 Windows 侧，防止 TX11 坐标同步导致的“鬼畜”跳变
                int flags = MouseEventFlags.getFlagFor(button, true);
                if (flags != 0) xServer.getWinHandler().mouseEvent(flags, 0, 0, 0);
            } else {
                // 桌面模式：走 TX11 侧，坐标为 (0,0) 配合 relative=true 表示“在当前位置点击”
                lorieView.sendMouseEvent(0, 0, btnCode, true, true);
            }
        }
    }

    @Override
    public void injectPointerButtonRelease(int btnCode) {
        Pointer.Button button = pointerBtnCode2Enum(btnCode);
        xServer.injectPointerButtonRelease(button);
        if (btnCode == 4 || btnCode == 5) return;

        if (xServer.isRelativeMouseMovement()) {
            int flags = MouseEventFlags.getFlagFor(button, false);
            if (flags != 0) xServer.getWinHandler().mouseEvent(flags, 0, 0, 0);
        } else {
            lorieView.sendMouseEvent(0, 0, btnCode, false, true);
        }
    }

    @Override
    public boolean isPointerButtonPressed(int btnCode) {
        return xServer.pointer.isButtonPressed(pointerBtnCode2Enum(btnCode));
    }

    @Override
    public void injectKeyPress(byte keycode, int keysym) {
        lorieView.sendKeyEvent(keycode - 8, keycode, true);
    }

    @Override
    public void injectKeyRelease(byte keycode) {
        lorieView.sendKeyEvent(keycode - 8, keycode, false);
    }

    private Pointer.Button pointerBtnCode2Enum(int btnCode) {
        return Pointer.Button.values()[btnCode - 1];
    }
}
