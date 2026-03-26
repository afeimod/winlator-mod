package com.winlator.xserver.extensions;

import static com.winlator.xserver.XClientRequestHandler.RESPONSE_CODE_ERROR;
import static com.winlator.xserver.XClientRequestHandler.RESPONSE_CODE_SUCCESS;

import android.util.SparseArray;
import android.util.SparseLongArray;

import androidx.annotation.Keep;

import com.winlator.core.Callback;
import com.winlator.renderer.Texture;
import com.winlator.xconnector.XInputStream;
import com.winlator.xconnector.XOutputStream;
import com.winlator.xconnector.XStreamLock;
import com.winlator.xserver.Drawable;
import com.winlator.xserver.Window;
import com.winlator.xserver.XClient;
import com.winlator.xserver.XServer;
import com.winlator.xserver.errors.BadAlloc;
import com.winlator.xserver.errors.BadImplementation;
import com.winlator.xserver.errors.GLXBadContext;
import com.winlator.xserver.errors.GLXBadFBConfig;
import com.winlator.xserver.errors.XRequestError;

import java.io.IOException;

public class GLXExtension extends Extension {
    public static final byte MAJOR_VERSION = 1;
    public static final byte MINOR_VERSION = 4;
    private static final byte DEFAULT_FBCONFIG_ID = 1;
    private final SparseArray<SparseLongArray> clientGLXContexts = new SparseArray<>();
    private final SparseArray<SparseLongArray> clientGLContexts = new SparseArray<>();
    private final String glxExtensions = "GLX_ARB_create_context GLX_ARB_get_proc_address";
    private final Callback<XClient> onDestroyClientListener = (client) -> {
        destroyAllGLContexts(client.fd);
        destroyAllGLXContexts(client.fd);
    };

    static {
        System.loadLibrary("gladiorenderer");
    }

    public GLXExtension(XServer xServer, byte majorOpcode) {
        super(xServer, majorOpcode);
    }

    private static abstract class ClientOpcodes {
        private static final byte CREATE_GL_CONTEXT = 1;
        private static final byte DESTROY_GL_CONTEXT = 2;
        private static final byte CREATE_CONTEXT = 3;
        private static final byte DESTROY_CONTEXT = 4;
        private static final byte QUERY_VERSION = 7;
        private static final byte QUERY_EXTENSIONS_STRING = 18;
        private static final byte QUERY_SERVER_STRING = 19;
        private static final byte GET_FB_CONFIGS = 21;
        private static final byte CREATE_CONTEXT_ATTRIBS_ARB = 34;
    }

    @Override
    public String getName() {
        return "GLX";
    }

    @Override
    public byte getFirstErrorId() {
        return Byte.MIN_VALUE;
    }

    private void createGLContext(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int contextId = inputStream.readInt();

        SparseLongArray contexts = clientGLContexts.get(client.fd);
        if (contexts == null) {
            clientGLContexts.put(client.fd, contexts = new SparseLongArray());
            client.addOnDestroyListener(onDestroyClientListener);
        }

        long context = createGLContext(client.fd);
        if (context != 0) contexts.put(contextId, context);
    }

    private void destroyGLContext(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int contextId = inputStream.readInt();

        SparseLongArray contexts = clientGLContexts.get(client.fd);
        if (contexts == null) throw new GLXBadContext();

        long context = contexts.get(contextId, 0L);
        if (context != 0) destroyGLContext(context);
        contexts.delete(contextId);

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writePad(28);
        }
    }

    private void createGLXContextForClient(XClient client, int contextId, int shareContextId) throws IOException, XRequestError {
        synchronized (clientGLXContexts) {
            SparseLongArray contexts = clientGLXContexts.get(client.fd);
            if (contexts == null) {
                clientGLXContexts.put(client.fd, contexts = new SparseLongArray());
                client.addOnDestroyListener(onDestroyClientListener);
            }

            long sharedContextPtr = shareContextId > 0 ? contexts.get(shareContextId) : 0;
            long context = createGLXContext(contextId, sharedContextPtr);
            if (context == 0) throw new BadAlloc();
            contexts.put(contextId, context);
        }
    }

    private void createContext(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int contextId = inputStream.readInt();
        inputStream.skip(8);
        int shareList = inputStream.readInt();
        boolean isDirect = inputStream.readByte() == 1;

        if (contextId == 0) throw new GLXBadContext();
        createGLXContextForClient(client, contextId, shareList);

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writePad(28);
        }
    }

    private void destroyContext(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int contextId = inputStream.readInt();

        synchronized (clientGLXContexts) {
            SparseLongArray contexts = clientGLXContexts.get(client.fd);
            if (contexts == null) throw new GLXBadContext();

            long context = contexts.get(contextId);
            if (context == 0) throw new GLXBadContext();

            destroyGLXContext(context);
            contexts.delete(contextId);
        }
    }

    private void queryVersion(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
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

    private void queryExtensionsString(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        inputStream.skip(4);
        int length = glxExtensions.length();

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writeInt((length + (-length & 3)) / 4);
            outputStream.writeInt(0);
            outputStream.writeInt(length);
            outputStream.writePad(16);
            outputStream.writeString8(glxExtensions);
        }
    }

    private void queryServerString(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        inputStream.skip(4);
        int name = inputStream.readInt();

        String string = "";

        switch (name) {
            case GLXEnums.GLX_VENDOR:
                string = "Winlator";
                break;
            case GLXEnums.GLX_VERSION:
                string = MAJOR_VERSION+"."+MINOR_VERSION;
                break;
            case GLXEnums.GLX_EXTENSIONS:
                string = glxExtensions;
                break;
        }
        int length = string.length();

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writeInt((length + (-length & 3)) / 4);
            outputStream.writeInt(0);
            outputStream.writeInt(length);
            outputStream.writePad(16);
            outputStream.writeString8(string);
        }
    }

    private void getFBConfigs(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        inputStream.skip(4);

        final int numFBConfigs = 1;
        final int numProperties = 11;
        final int[] properties = new int[]{
            GLXEnums.GLX_FBCONFIG_ID, DEFAULT_FBCONFIG_ID,
            GLXEnums.GLX_RED_SIZE, 8,
            GLXEnums.GLX_GREEN_SIZE, 8,
            GLXEnums.GLX_BLUE_SIZE, 8,
            GLXEnums.GLX_ALPHA_SIZE, 8,
            GLXEnums.GLX_DEPTH_SIZE, 24,
            GLXEnums.GLX_STENCIL_SIZE, 8,
            GLXEnums.GLX_BUFFER_SIZE, 32,
            GLXEnums.GLX_DOUBLEBUFFER, 1,
            GLXEnums.GLX_DRAWABLE_TYPE, GLXEnums.GLX_WINDOW_BIT,
            GLXEnums.GLX_RENDER_TYPE, GLXEnums.GLX_RGBA_BIT
        };

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(RESPONSE_CODE_SUCCESS);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writeInt(2 * numFBConfigs * numProperties);
            outputStream.writeInt(numFBConfigs);
            outputStream.writeInt(numProperties);
            outputStream.writePad(16);

            for (int i = 0, j, k = 0; i < numFBConfigs; i++) {
                for (j = 0; j < numProperties; j++, k++) {
                    outputStream.writeInt(properties[k*2+0]);
                    outputStream.writeInt(properties[k*2+1]);
                }
            }
        }
    }

    private void createContextAttribsARB(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int contextId = inputStream.readInt();
        int fbConfigId = inputStream.readInt();
        inputStream.skip(4);
        int shareContext = inputStream.readInt();
        inputStream.skip(4);
        int numAttribs = inputStream.readInt();

        if (contextId == 0) throw new GLXBadContext();
        if (fbConfigId != DEFAULT_FBCONFIG_ID) throw new GLXBadFBConfig();

        int glMajorVersion = 3;
        int glMinorVersion = 3;
        for (int i = 0; i < numAttribs; i++) {
            int name = inputStream.readInt();
            int value = inputStream.readInt();

            if (name == GLXEnums.GLX_CONTEXT_MAJOR_VERSION_ARB) glMajorVersion = value;
            else if (name == GLXEnums.GLX_CONTEXT_MINOR_VERSION_ARB) glMinorVersion = value;
        }

        boolean success = glMajorVersion <= 3 && glMinorVersion <= 3;
        if (success) createGLXContextForClient(client, contextId, shareContext);

        try (XStreamLock lock = outputStream.lock()) {
            outputStream.writeByte(success ? RESPONSE_CODE_SUCCESS : RESPONSE_CODE_ERROR);
            outputStream.writeByte((byte)0);
            outputStream.writeShort(client.getSequenceNumber());
            outputStream.writePad(28);
        }
    }

    @Override
    public void handleRequest(XClient client, XInputStream inputStream, XOutputStream outputStream) throws IOException, XRequestError {
        int opcode = client.getRequestData();
        switch (opcode) {
            case ClientOpcodes.CREATE_GL_CONTEXT:
                createGLContext(client, inputStream, outputStream);
                break;
            case ClientOpcodes.DESTROY_GL_CONTEXT:
                destroyGLContext(client, inputStream, outputStream);
                break;
            case ClientOpcodes.CREATE_CONTEXT:
                createContext(client, inputStream, outputStream);
                break;
            case ClientOpcodes.DESTROY_CONTEXT:
                destroyContext(client, inputStream, outputStream);
                break;
            case ClientOpcodes.QUERY_VERSION:
                queryVersion(client, inputStream, outputStream);
                break;
            case ClientOpcodes.QUERY_EXTENSIONS_STRING:
                queryExtensionsString(client, inputStream, outputStream);
                break;
            case ClientOpcodes.QUERY_SERVER_STRING:
                queryServerString(client, inputStream, outputStream);
                break;
            case ClientOpcodes.GET_FB_CONFIGS:
                getFBConfigs(client, inputStream, outputStream);
                break;
            case ClientOpcodes.CREATE_CONTEXT_ATTRIBS_ARB:
                createContextAttribsARB(client, inputStream, outputStream);
                break;
            default:
                throw new BadImplementation();
        }
    }

    @Keep
    private short[] getWindowSize(int windowId) {
        Window window = xServer.windowManager.getWindow(windowId);
        return window != null ? new short[]{window.getWidth(), window.getHeight()} : new short[]{0, 0};
    }

    @Keep
    private void clearWindowContent(int windowId) {
        Window window = xServer.windowManager.getWindow(windowId);
        if (window != null) {
            Drawable drawable = window.getContent();
            if (drawable.getData() != null) {
                drawable.setData(null);
                drawable.getTexture().destroy();
            }
        }
    }

    @Keep
    private boolean updateWindowContent(int drawableId, short width, short height, boolean flipY) {
        Drawable drawable = xServer.drawableManager.getDrawable(drawableId);
        if (drawable == null) return true;

        synchronized (drawable.renderLock) {
            if (drawable.width != width || drawable.height != height) return false;

            drawable.setData(null);
            Texture texture = drawable.getTexture();
            texture.setFlipY(flipY);
            texture.copyFromReadBuffer(width, height);
            Runnable onDrawListener = drawable.getOnDrawListener();
            if (onDrawListener != null) onDrawListener.run();
        }
        return true;
    }

    @Keep
    private long getGLXContextPtr(int clientFd, int id) {
        synchronized (clientGLXContexts) {
            SparseLongArray contexts = clientGLXContexts.get(clientFd);
            return contexts != null ? contexts.get(id) : 0;
        }
    }

    private void destroyAllGLContexts(int clientFd) {
        synchronized (clientGLContexts) {
            SparseLongArray contexts = clientGLContexts.get(clientFd);
            if (contexts != null) {
                for (int i = 0; i < contexts.size(); i++) destroyGLContext(contexts.valueAt(i));
                contexts.clear();
            }
            clientGLContexts.remove(clientFd);
        }
    }

    private void destroyAllGLXContexts(int clientFd) {
        synchronized (clientGLXContexts) {
            SparseLongArray contexts = clientGLXContexts.get(clientFd);
            if (contexts != null) {
                for (int i = 0; i < contexts.size(); i++) destroyGLXContext(contexts.valueAt(i));
                contexts.clear();
            }
            clientGLXContexts.remove(clientFd);
        }
    }

    private native long createGLContext(int clientFd);

    private native void destroyGLContext(long contextPtr);

    private native long createGLXContext(int contextId, long sharedContextPtr);

    private native void destroyGLXContext(long contextPtr);
}
