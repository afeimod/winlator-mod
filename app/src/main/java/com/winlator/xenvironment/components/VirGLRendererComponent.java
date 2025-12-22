package com.winlator.xenvironment.components;

import android.opengl.GLES20;

import androidx.annotation.Keep;

import com.winlator.renderer.Texture;
import com.winlator.xconnector.ConnectedClient;
import com.winlator.xconnector.ConnectionHandler;
import com.winlator.xconnector.RequestHandler;
import com.winlator.xconnector.UnixSocketConfig;
import com.winlator.xconnector.XConnectorEpoll;
import com.winlator.xenvironment.EnvironmentComponent;
import com.winlator.xserver.Drawable;
import com.winlator.xserver.XServer;

import java.io.IOException;

public class VirGLRendererComponent extends EnvironmentComponent implements ConnectionHandler, RequestHandler {
    private final XServer xServer;
    private final UnixSocketConfig socketConfig;
    private XConnectorEpoll connector;

    static {
        System.loadLibrary("virglrenderer");
    }

    public VirGLRendererComponent(XServer xServer, UnixSocketConfig socketConfig) {
        this.xServer = xServer;
        this.socketConfig = socketConfig;
    }

    @Override
    public void start() {
        if (connector != null) return;
        connector = new XConnectorEpoll(socketConfig, this, this);
        connector.setInitialInputBufferCapacity(0);
        connector.setInitialOutputBufferCapacity(0);
        connector.start();
    }

    @Override
    public void stop() {
        if (connector != null) {
            connector.destroy();
            connector = null;
        }
    }

    @Keep
    private void killConnection(int fd) {
        connector.killConnection(connector.getClientWidthFd(fd));
    }

    @Override
    public void handleConnectionShutdown(ConnectedClient client) {
        long clientPtr = (long)client.getTag();
        destroyClient(clientPtr);
    }

    @Override
    public void handleNewConnection(ConnectedClient client) {
        long clientPtr = handleNewConnection(client.fd);
        client.setTag(clientPtr);
    }

    @Override
    public boolean handleRequest(ConnectedClient client) throws IOException {
        long clientPtr = (long)client.getTag();
        handleRequest(clientPtr);
        return true;
    }

    @Keep
    private void flushFrontbuffer(int drawableId, int framebuffer) {
        Drawable drawable = xServer.drawableManager.getDrawable(drawableId);
        if (drawable == null) return;

        synchronized (drawable.renderLock) {
            drawable.setData(null);
            Texture texture = drawable.getTexture();
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, framebuffer);
            texture.copyFromReadBuffer(drawable.width, drawable.height);
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        }

        Runnable onDrawListener = drawable.getOnDrawListener();
        if (onDrawListener != null) onDrawListener.run();
    }

    private native long handleNewConnection(int fd);

    private native void handleRequest(long clientPtr);

    private native long getCurrentEGLContextPtr();

    private native void destroyClient(long clientPtr);

    private native void destroyRenderer(long clientPtr);
}
