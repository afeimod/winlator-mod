package com.winlator.xserver.extensions;

import static com.winlator.xserver.XClientRequestHandler.RESPONSE_CODE_SUCCESS;

import com.winlator.xconnector.XInputStream;
import com.winlator.xconnector.XOutputStream;
import com.winlator.xconnector.XStreamLock;
import com.winlator.xserver.Window;
import com.winlator.xserver.XClient;
import com.winlator.xserver.errors.BadAccess;
import com.winlator.xserver.errors.BadImplementation;
import com.winlator.xserver.errors.BadMatch;
import com.winlator.xserver.errors.BadValue;
import com.winlator.xserver.errors.BadWindow;
import com.winlator.xserver.errors.XRequestError;

import java.io.IOException;

public class XComposite implements Extension {
    public static final byte MAJOR_OPCODE = -105;
    public static final byte MAJOR_VERSION = 0;
    public static final byte MINOR_VERSION = 1;
    public enum UpdateMode {REDIRECT_AUTOMATIC, REDIRECT_MANUAL}

    private static abstract class ClientOpcodes {
        private static final byte QUERY_VERSION = 0;
        private static final byte REDIRECT_WINDOW = 1;
        private static final byte UNREDIRECT_WINDOW = 3;
    }

    @Override
    public String getName() {
        return "Composite";
    }

    @Override
    public byte getMajorOpcode() {
        return MAJOR_OPCODE;
    }

    @Override
    public byte getFirstErrorId() {
        return 0;
    }

    @Override
    public byte getFirstEventId() {
        return 0;
    }

    private static void setWindowsToOffscreenStorage(Window window, boolean offscreenStorage) {
        if (!window.attributes.isMapped()) return;
        window.getContent().setOffscreenStorage(offscreenStorage);

        for (Window child : window.getChildren()) {
            setWindowsToOffscreenStorage(child, offscreenStorage);
        }
    }

    private static void queryVersion(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        inputStream.skip(8);

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writeInt(0);
            outputStream.writeInt(MAJOR_VERSION);
            outputStream.writeInt(MINOR_VERSION);
            outputStream.writePad(16);
        }
    }

    private static void redirectWindow(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int windowId = inputStream.readInt();
        byte updateMode = inputStream.readByte();
        inputStream.skip(3);

        Window window = client.xServer.windowManager.getWindow(windowId);
        if (window == null) throw new BadWindow(windowId);

        if (window == client.xServer.windowManager.rootWindow) throw new BadMatch();
        if (updateMode != UpdateMode.REDIRECT_MANUAL.ordinal()) throw new BadImplementation();
        if (window.getTag("compositeRedirectParent") != null) throw new BadAccess();

        Window parent = window.getParent();
        window.setTag("compositeRedirectParent", parent);
        setWindowsToOffscreenStorage(window, true);
        parent.attributes.setRenderSubwindows(false);
        client.xServer.windowManager.triggerOnChangeWindowZOrder(window);
    }

    private static void unredirectWindow(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int windowId = inputStream.readInt();
        byte updateMode = inputStream.readByte();
        inputStream.skip(3);

        Window window = client.xServer.windowManager.getWindow(windowId);
        if (window == null) throw new BadWindow(windowId);

        if (window == client.xServer.windowManager.rootWindow) throw new BadMatch();
        if (updateMode != UpdateMode.REDIRECT_MANUAL.ordinal()) throw new BadImplementation();

        Window oldParent = (Window)window.getTag("compositeRedirectParent");
        if (oldParent == null) throw new BadValue(windowId);

        window.removeTag("compositeRedirectParent");
        setWindowsToOffscreenStorage(window, false);
        oldParent.attributes.setRenderSubwindows(true);
        client.xServer.windowManager.triggerOnChangeWindowZOrder(window);
    }

    @Override
    public void handleRequest(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int opcode = client.getRequestData();

        switch (opcode) {
            case ClientOpcodes.QUERY_VERSION :
                queryVersion(client, inputStream, outputStream);
                break;
            case ClientOpcodes.REDIRECT_WINDOW :
                redirectWindow(client, inputStream, outputStream);
                break;
            case ClientOpcodes.UNREDIRECT_WINDOW:
                unredirectWindow(client, inputStream, outputStream);
                break;
            default:
                throw new BadImplementation();
        }
    }
}
