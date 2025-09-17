package com.winlator.winhandler;

import android.content.Intent;
import android.net.Uri;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.winlator.XServerDisplayActivity;
import com.winlator.core.DefaultVersion;
import com.winlator.core.GeneralComponents;
import com.winlator.core.StringUtils;
import com.winlator.xserver.XServer;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayDeque;
import java.util.concurrent.Executors;

public class WinHandler {
    private static final short SERVER_PORT = 7947;
    private static final short CLIENT_PORT = 7946;
    private static final byte DEFAULT_PACKET_LENGTH = 64;
    private DatagramSocket socket;
    protected final ByteBuffer sendData = ByteBuffer.allocate(128).order(ByteOrder.LITTLE_ENDIAN);
    protected final ByteBuffer receiveData = ByteBuffer.allocate(DEFAULT_PACKET_LENGTH).order(ByteOrder.LITTLE_ENDIAN);
    private final DatagramPacket sendPacket = new DatagramPacket(sendData.array(), sendData.capacity());
    private final DatagramPacket receivePacket = new DatagramPacket(receiveData.array(), receiveData.capacity());
    private final ArrayDeque<Runnable> actions = new ArrayDeque<>();
    protected boolean initReceived = false;
    private boolean running = false;
    private OnGetProcessInfoListener onGetProcessInfoListener;
    private InetAddress localhost;
    protected final XServerDisplayActivity activity;
    private MIDIHandler midiHandler;
    public final GamepadHandler gamepadHandler = new GamepadHandler(this);

    public WinHandler(XServerDisplayActivity activity) {
        this.activity = activity;
    }

    protected boolean sendPacket(int port) {
        return sendPacket(port, DEFAULT_PACKET_LENGTH);
    }

    protected boolean sendPacket(int port, int packetLength) {
        try {
            int size = sendData.position();
            if (size == 0) return false;
            sendPacket.setAddress(localhost);
            sendPacket.setPort(port);
            sendPacket.setLength(packetLength);
            socket.send(sendPacket);
            sendPacket.setLength(DEFAULT_PACKET_LENGTH);
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }

    protected boolean sendPacket(int port, byte[] data) {
        try {
            sendPacket.setData(data);
            sendPacket.setAddress(localhost);
            sendPacket.setPort(port);
            socket.send(sendPacket);
            sendPacket.setData(sendData.array());
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }

    public void exec(String command) {
        command = command.trim();
        if (command.isEmpty()) return;
        String[] cmdList = command.split(" ", 2);
        final String filename = cmdList[0];
        final String parameters = cmdList.length > 1 ? cmdList[1] : "";

        addAction(() -> {
            byte[] filenameBytes = filename.getBytes();
            byte[] parametersBytes = parameters.getBytes();

            sendData.rewind();
            sendData.put(RequestCodes.EXEC);
            sendData.putInt(filenameBytes.length + parametersBytes.length + 8);
            sendData.putInt(filenameBytes.length);
            sendData.putInt(parametersBytes.length);
            sendData.put(filenameBytes);
            sendData.put(parametersBytes);
            sendPacket(CLIENT_PORT);
        });
    }

    public void killProcess(final String processName) {
        killProcess(processName, 0);
    }

    public void killProcess(final String processName, final int pid) {
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.KILL_PROCESS);
            if (processName != null) {
                byte[] bytes = processName.getBytes();
                int minLength = Math.min(bytes.length, 55);
                sendData.putInt(minLength);
                sendData.put(bytes, 0, minLength);
            }
            else sendData.putInt(0);
            sendData.putInt(pid);
            sendPacket(CLIENT_PORT);
        });
    }

    public void listProcesses() {
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.LIST_PROCESSES);
            sendData.putInt(0);

            if (!sendPacket(CLIENT_PORT) && onGetProcessInfoListener != null) {
                onGetProcessInfoListener.onGetProcessInfo(0, 0, null);
            }
        });
    }

    public void setProcessAffinity(final String processName, final int affinityMask) {
        addAction(() -> {
            byte[] bytes = processName.getBytes();
            sendData.rewind();
            sendData.put(RequestCodes.SET_PROCESS_AFFINITY);
            sendData.putInt(9 + bytes.length);
            sendData.putInt(0);
            sendData.putInt(affinityMask);
            sendData.put((byte)bytes.length);
            sendData.put(bytes);
            sendPacket(CLIENT_PORT);
        });
    }

    public void setProcessAffinity(final int pid, final int affinityMask) {
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.SET_PROCESS_AFFINITY);
            sendData.putInt(9);
            sendData.putInt(pid);
            sendData.putInt(affinityMask);
            sendData.put((byte)0);
            sendPacket(CLIENT_PORT);
        });
    }

    public void mouseEvent(int flags, int dx, int dy, int wheelDelta) {
        if (!initReceived) return;
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.MOUSE_EVENT);
            sendData.putInt(10);
            sendData.putInt(flags);
            sendData.putShort((short)dx);
            sendData.putShort((short)dy);
            sendData.putShort((short)wheelDelta);
            sendData.put((byte)((flags & MouseEventFlags.MOVE) != 0 ? 1 : 0)); // cursor pos feedback
            sendPacket(CLIENT_PORT);
        });
    }

    public void keyboardEvent(byte vkey, int flags) {
        if (!initReceived) return;
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.KEYBOARD_EVENT);
            sendData.put(vkey);
            sendData.putInt(flags);
            sendPacket(CLIENT_PORT);
        });
    }

    public void bringToFront(final String processName) {
        bringToFront(processName, 0);
    }

    public void bringToFront(final String processName, final long handle) {
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.BRING_TO_FRONT);
            byte[] bytes = processName.getBytes();
            int minLength = Math.min(bytes.length, 51);
            sendData.putInt(minLength);
            sendData.put(bytes, 0, minLength);
            sendData.putLong(handle);
            sendPacket(CLIENT_PORT);
        });
    }

    public void showDesktop() {
        addAction(() -> {
            sendData.rewind();
            sendData.put(RequestCodes.SHOW_DESKTOP);
            sendData.putInt(0);
            sendPacket(CLIENT_PORT);
        });
    }

    public void setClipboardData(final String data) {
        addAction(() -> {
            sendData.rewind();
            byte[] bytes = data.getBytes();
            sendData.put(RequestCodes.SET_CLIPBOARD_DATA);
            sendData.putInt(bytes.length);

            if (sendPacket(CLIENT_PORT)) sendPacket(CLIENT_PORT, bytes);
        });
    }

    protected void addAction(Runnable action) {
        synchronized (actions) {
            actions.add(action);
            actions.notify();
        }
    }

    public OnGetProcessInfoListener getOnGetProcessInfoListener() {
        return onGetProcessInfoListener;
    }

    public void setOnGetProcessInfoListener(OnGetProcessInfoListener onGetProcessInfoListener) {
        synchronized (actions) {
            this.onGetProcessInfoListener = onGetProcessInfoListener;
        }
    }

    private void startSendThread() {
        Executors.newSingleThreadExecutor().execute(() -> {
            while (running) {
                synchronized (actions) {
                    while (initReceived && !actions.isEmpty()) actions.poll().run();
                    try {
                        actions.wait();
                    }
                    catch (InterruptedException e) {}
                }
            }
        });
    }

    public void stop() {
        running = false;

        if (socket != null) {
            socket.close();
            socket = null;
        }

        synchronized (actions) {
            actions.notify();
        }

        if (midiHandler != null) {
            midiHandler.close();
            midiHandler.destroy();
            midiHandler = null;
        }
    }

    private void handleRequest(byte requestCode, final int port) throws IOException {
        switch (requestCode) {
            case RequestCodes.INIT: {
                initReceived = true;

                synchronized (actions) {
                    actions.notify();
                }
                break;
            }
            case RequestCodes.GET_PROCESS: {
                if (onGetProcessInfoListener == null) return;
                receiveData.position(receiveData.position() + 4);
                int numProcesses = receiveData.getShort();
                int index = receiveData.getShort();
                int pid = receiveData.getInt();
                long memoryUsage = receiveData.getLong();
                int affinityMask = receiveData.getInt();
                boolean wow64Process = receiveData.get() == 1;

                byte[] bytes = new byte[32];
                receiveData.get(bytes);
                String name = StringUtils.fromANSIString(bytes);

                onGetProcessInfoListener.onGetProcessInfo(index, numProcesses, new ProcessInfo(pid, name, memoryUsage, affinityMask, wow64Process));
                break;
            }
            case RequestCodes.GET_GAMEPAD: {
                gamepadHandler.handleGetGamepadRequest(port);
                break;
            }
            case RequestCodes.RELEASE_GAMEPAD: {
                gamepadHandler.handleReleaseGamepadRequest(port);
                break;
            }
            case RequestCodes.SET_GAMEPAD_STATE: {
                gamepadHandler.handleSetGamepadStateRequest(port);
                break;
            }
            case RequestCodes.CURSOR_POS_FEEDBACK: {
                short x = receiveData.getShort();
                short y = receiveData.getShort();
                XServer xServer = activity.getXServer();
                xServer.pointer.setX(x);
                xServer.pointer.setY(y);
                activity.getXServerView().requestRender();
                break;
            }
            case RequestCodes.OPEN_URL: {
                int requestLength = receiveData.getInt();
                byte[] data = new byte[requestLength];
                socket.receive(new DatagramPacket(data, data.length));
                Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(new String(data)));
                intent.addFlags(Intent.FLAG_ACTIVITY_LAUNCH_ADJACENT | Intent.FLAG_ACTIVITY_NEW_TASK);
                activity.startActivity(intent);
                break;
            }
            case RequestCodes.MIDI_OPEN: {
                boolean isMidiOut = receiveData.get() == 1;
                if (midiHandler == null) midiHandler = new MIDIHandler(this);
                if (isMidiOut) {
                    midiHandler.open(() -> {
                        if (midiHandler.init()) {
                            String soundfont = activity.getPreferences().getString("soundfont", DefaultVersion.SOUNDFONT);
                            midiHandler.loadSoundFont(GeneralComponents.getDefinitivePath(GeneralComponents.Type.SOUNDFONT, activity, soundfont));
                        }
                    });
                }
                else {
                    midiHandler.outputPortConnect();
                    midiHandler.addClient(port);
                }
                break;
            }
            case RequestCodes.MIDI_CLOSE: {
                if (midiHandler != null) {
                    midiHandler.outputPortDisconnect();
                    midiHandler.close();
                }
                break;
            }
        }
    }

    public void start() {
        try {
            localhost = InetAddress.getLocalHost();
        }
        catch (UnknownHostException e) {
            try {
                localhost = InetAddress.getByName("127.0.0.1");
            }
            catch (UnknownHostException ex) {}
        }

        running = true;
        startSendThread();
        Executors.newSingleThreadExecutor().execute(() -> {
            try {
                socket = new DatagramSocket(null);
                socket.setReuseAddress(true);
                socket.bind(new InetSocketAddress((InetAddress)null, SERVER_PORT));

                while (running) {
                    socket.receive(receivePacket);

                    synchronized (actions) {
                        receiveData.rewind();
                        byte requestCode = receiveData.get();
                        handleRequest(requestCode, receivePacket.getPort());
                    }
                }
            }
            catch (IOException e) {}
        });
    }

    public boolean onGenericMotionEvent(MotionEvent event) {
        return gamepadHandler.onGenericMotionEvent(event);
    }

    public boolean onKeyEvent(KeyEvent event) {
        return gamepadHandler.onKeyEvent(event);
    }

    public MIDIHandler getMIDIhandler() {
        if (midiHandler == null) midiHandler = new MIDIHandler(this);
        return midiHandler;
    }
}
