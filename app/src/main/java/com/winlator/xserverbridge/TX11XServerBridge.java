package com.winlator.xserverbridge;

import android.util.Log;

import com.termux.x11.LorieView;
import com.winlator.winhandler.MouseEventFlags;
import com.winlator.winhandler.WinHandler;
import com.winlator.xserver.Pointer;
import com.winlator.xserver.XServer;

// tx11 没有获取当前指针状态的方法，所以还是要借助原 xserver 的 pointer 保存状态，用于读取
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
        // TODO 不知道这么写有没有问题
        if (xServer.isRelativeMouseMovement()) {
            xServer.getWinHandler().mouseEvent(MouseEventFlags.MOVE, dx, dy, 0);
        } else {
            lorieView.sendMouseEvent(dx, dy, 0, false, true);
        }
    }

    @Override
    public void injectPointerButtonPress(int btnCode) {
        xServer.injectPointerButtonPress(pointerBtnCode2Enum(btnCode));
        // 滚轮
        if (btnCode == 4 || btnCode == 5) {
            lorieView.sendMouseEvent(0, btnCode == 4 ? -60 : 60, 4, false, true);
        } else {
            lorieView.sendMouseEvent(0, 0, btnCode, true, xServer.isRelativeMouseMovement());
        }
    }

    @Override
    public void injectPointerButtonRelease(int btnCode) {
        xServer.injectPointerButtonRelease(pointerBtnCode2Enum(btnCode));
        // 滚轮
        if (btnCode == 4 || btnCode == 5) {
            return;
        }
        lorieView.sendMouseEvent(0, 0, btnCode, false, xServer.isRelativeMouseMovement());
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
