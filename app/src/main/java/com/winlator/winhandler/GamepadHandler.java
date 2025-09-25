package com.winlator.winhandler;

import android.content.SharedPreferences;
import android.view.KeyEvent;
import android.view.MotionEvent;

import com.winlator.core.ArrayUtils;
import com.winlator.inputcontrols.ControlsProfile;
import com.winlator.inputcontrols.ExternalController;
import com.winlator.inputcontrols.GamepadSlot;
import com.winlator.inputcontrols.GamepadState;
import com.winlator.inputcontrols.GamepadVibration;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

public class GamepadHandler {
    public enum PreferredInputApi {AUTO, DINPUT, XINPUT, BOTH}
    public static final byte DINPUT_MAPPER_TYPE_STANDARD = 0;
    public static final byte DINPUT_MAPPER_TYPE_XINPUT = 1;
    private static final byte GAMEPAD_MAX_COUNT = 4;
    private static final short DINPUT_PACKET_LENGTH = 128;
    private static final short XINPUT_PACKET_LENGTH = 16;
    private final WinHandler winHandler;
    private final List<GamepadClient> gamepadClients = new CopyOnWriteArrayList<>();
    private PreferredInputApi preferredInputApi = PreferredInputApi.AUTO;
    private byte dinputMapperType = DINPUT_MAPPER_TYPE_XINPUT;
    private final GamepadSlot[] gamepadSlots = new GamepadSlot[GAMEPAD_MAX_COUNT];
    private final ArrayList<ExternalController> connectedControllers = new ArrayList<>(GAMEPAD_MAX_COUNT);
    private GamepadPlayerConfig[] gamepadPlayerConfigs;

    private static class GamepadClient {
        private final int port;
        private final int processId;
        private final boolean isXInput;
        private final boolean[] enabledSlots = new boolean[GAMEPAD_MAX_COUNT];
        private boolean updatedOnce = false;

        private GamepadClient(int port, int processId, boolean isXInput) {
            this.port = port;
            this.processId = processId;
            this.isXInput = isXInput;
        }
    }

    public GamepadHandler(WinHandler winHandler) {
        this.winHandler = winHandler;
    }

    private void updateGamepadSlots() {
        if (gamepadPlayerConfigs == null) {
            SharedPreferences preferences = winHandler.activity.getPreferences();
            gamepadPlayerConfigs = new GamepadPlayerConfig[GAMEPAD_MAX_COUNT];
            for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
                gamepadPlayerConfigs[i] = new GamepadPlayerConfig(preferences.getString("gamepad_player"+i, ""));
            }
        }

        ControlsProfile profile = winHandler.activity.getInputControlsView().getProfile();
        boolean useVirtualGamepad = profile != null && profile.isVirtualGamepad();

        for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) gamepadSlots[i] = null;

        synchronized (connectedControllers) {
            ExternalController.updateConnectedControllers(connectedControllers);
        }

        boolean autoAssign = true;
        for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
            GamepadPlayerConfig config = gamepadPlayerConfigs[i];
            if (config.name.isEmpty()) continue;
            if (config.mode == GamepadPlayerConfig.MODE_EXTERNAL_CONTROLLER) {
                for (ExternalController controller : connectedControllers) {
                    if (controller.getName().equals(config.name)) {
                        gamepadSlots[i] = controller;
                        autoAssign = false;
                        break;
                    }
                }
            }
            else if (useVirtualGamepad && profile.getName().equals(config.name)) {
                gamepadSlots[i] = profile;
                autoAssign = false;
            }
        }

        if (autoAssign) {
            if (useVirtualGamepad) gamepadSlots[0] = profile;
            int index = 0;
            for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
                if (gamepadSlots[i] != null) continue;
                gamepadSlots[i] = index < connectedControllers.size() ? connectedControllers.get(index) : null;
                index++;
            }
        }
    }

    private boolean isAnyGamepadConnected() {
        for (GamepadSlot gamepadSlot : gamepadSlots) if (gamepadSlot != null) return true;
        return false;
    }

    public void handleGetGamepadRequest(int port) {
        boolean isXInput = winHandler.receiveData.get() == 1;
        boolean notify = winHandler.receiveData.get() == 1;
        int processId = winHandler.receiveData.getInt();
        boolean updatedOnce = winHandler.receiveData.get() == 1;

        updateGamepadSlots();

        if (!isAnyGamepadConnected() ||
            (preferredInputApi == PreferredInputApi.DINPUT && isXInput) || (preferredInputApi == PreferredInputApi.XINPUT && !isXInput)) {
            notify = false;
        }

        final boolean[] forceDisable = !isXInput ? new boolean[GAMEPAD_MAX_COUNT] : null;
        int clientIndex = findGamepadClientIndex(port, null, null);
        if (notify) {
            GamepadClient client;
            if (clientIndex == -1) {
                gamepadClients.add((client = new GamepadClient(port, processId, isXInput)));
            }
            else client = gamepadClients.get(clientIndex);
            client.updatedOnce = updatedOnce;

            for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) client.enabledSlots[i] = winHandler.receiveData.get() == 1;

            if (!isXInput && preferredInputApi == PreferredInputApi.AUTO) {
                int xinputClientIndex = findGamepadClientIndex(null, processId, true);

                if (xinputClientIndex != -1) {
                    GamepadClient xinputClient = gamepadClients.get(xinputClientIndex);
                    if (xinputClient.updatedOnce) {
                        for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
                            if (xinputClient.enabledSlots[i]) {
                                client.enabledSlots[i] = false;
                                forceDisable[i] = true;
                            }
                        }
                    }
                }
            }
        }
        else if (clientIndex != -1) gamepadClients.remove(clientIndex);

        winHandler.addAction(() -> {
            if (isXInput) {
                winHandler.sendData.rewind();
                winHandler.sendData.put(RequestCodes.GET_GAMEPAD);

                for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
                    winHandler.sendData.put((byte)(gamepadSlots[i] != null ? 1 : 0));
                    winHandler.sendData.put((byte)(gamepadPlayerConfigs != null && gamepadPlayerConfigs[i].vibration ? 1 : 0));
                }

                winHandler.sendPacket(port, XINPUT_PACKET_LENGTH);
            }
            else {
                winHandler.sendData.rewind();
                winHandler.sendData.put(RequestCodes.GET_GAMEPAD);
                winHandler.sendData.put(dinputMapperType);

                for (byte i = 0; i < GAMEPAD_MAX_COUNT; i++) {
                    if (gamepadSlots[i] != null && !forceDisable[i]) {
                        String name = gamepadSlots[i].getName();
                        byte[] bytes = name.getBytes();
                        byte nameLength = (byte)Math.min((byte)bytes.length, 31);
                        winHandler.sendData.put(nameLength);
                        winHandler.sendData.put(bytes, 0, nameLength);
                    }
                    else winHandler.sendData.put((byte)0);
                }

                winHandler.sendPacket(port, DINPUT_PACKET_LENGTH);
            }
        });
    }

    public void sendGamepadState(final GamepadSlot gamepadSlot) {
        if (!winHandler.initReceived || gamepadClients.isEmpty()) return;
        final byte slot = (byte)ArrayUtils.indexOf(gamepadSlots, gamepadSlot);
        if (slot == ArrayUtils.INDEX_NOT_FOUND) return;
        final GamepadState state = gamepadSlot.getGamepadState();

        for (final GamepadClient client : gamepadClients) {
            if (!client.enabledSlots[slot]) continue;
            winHandler.addAction(() -> {
                int packetLength = client.isXInput ? XINPUT_PACKET_LENGTH : DINPUT_PACKET_LENGTH;
                winHandler.sendData.rewind();
                winHandler.sendData.put(RequestCodes.GET_GAMEPAD_STATE);
                winHandler.sendData.put(slot);
                state.writeTo(winHandler.sendData);
                winHandler.sendPacket(client.port, packetLength);
            });
        }
    }

    public void handleReleaseGamepadRequest(int port) {
        int index = findGamepadClientIndex(port, null, null);
        if (index != -1) gamepadClients.remove(index);
    }

    public void handleSetGamepadStateRequest(int port) {
        byte slot = winHandler.receiveData.get();
        if (slot < 0 || slot >= GAMEPAD_MAX_COUNT) return;
        int leftMotorSpeed = winHandler.receiveData.getInt();
        int rightMotorSpeed = winHandler.receiveData.getInt();
        GamepadSlot gamepadSlot = gamepadSlots[slot];
        if (gamepadSlot == null) return;

        GamepadVibration vibration = gamepadSlot.getGamepadVibration();
        vibration.vibrate(leftMotorSpeed, rightMotorSpeed);
    }

    private int findGamepadClientIndex(Integer port, Integer processId, Boolean isXInput) {
        for (int i = 0; i < gamepadClients.size(); i++) {
            GamepadClient client = gamepadClients.get(i);
            if ((port == null || client.port == port) &&
                (isXInput == null || client.isXInput == isXInput) &&
                (processId == null || client.processId == processId)) return i;
        }
        return -1;
    }

    private ExternalController getConnectedControllerById(int deviceId) {
        synchronized (connectedControllers) {
            for (ExternalController controller : connectedControllers) {
                if (controller.getDeviceId() == deviceId) return controller;
            }

            return null;
        }
    }

    protected boolean onGenericMotionEvent(MotionEvent event) {
        boolean handled = false;
        ExternalController controller = getConnectedControllerById(event.getDeviceId());
        if (controller != null) {
            handled = controller.updateStateFromMotionEvent(event);
            if (handled) sendGamepadState(controller);
        }
        return handled;
    }

    protected boolean onKeyEvent(KeyEvent event) {
        boolean handled = false;
        ExternalController controller = getConnectedControllerById(event.getDeviceId());
        if (controller != null && event.getRepeatCount() == 0) {
            int action = event.getAction();

            if (action == KeyEvent.ACTION_DOWN) {
                handled = controller.updateStateFromKeyEvent(event);
            }
            else if (action == KeyEvent.ACTION_UP) {
                handled = controller.updateStateFromKeyEvent(event);
            }

            if (handled) sendGamepadState(controller);
        }
        return handled;
    }

    public byte getDInputMapperType() {
        return dinputMapperType;
    }

    public void setDInputMapperType(byte dinputMapperType) {
        this.dinputMapperType = dinputMapperType;
    }

    public PreferredInputApi getPreferredInputApi() {
        return preferredInputApi;
    }

    public void setPreferredInputApi(PreferredInputApi preferredInputApi) {
        this.preferredInputApi = preferredInputApi;
    }
}
