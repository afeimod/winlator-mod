package com.termux.x11;

import static android.view.KeyEvent.ACTION_MULTIPLE;
import static android.view.KeyEvent.KEYCODE_2;
import static android.view.KeyEvent.KEYCODE_3;
import static android.view.KeyEvent.KEYCODE_8;
import static android.view.KeyEvent.KEYCODE_ALT_RIGHT;
import static android.view.KeyEvent.KEYCODE_AT;
import static android.view.KeyEvent.KEYCODE_ENTER;
import static android.view.KeyEvent.KEYCODE_EQUALS;
import static android.view.KeyEvent.KEYCODE_ESCAPE;
import static android.view.KeyEvent.KEYCODE_PLUS;
import static android.view.KeyEvent.KEYCODE_POUND;
import static android.view.KeyEvent.KEYCODE_SHIFT_LEFT;
import static android.view.KeyEvent.KEYCODE_STAR;
import static android.view.KeyEvent.META_ALT_RIGHT_ON;
import static java.nio.charset.StandardCharsets.UTF_8;

import android.view.KeyEvent;

import java.util.TreeSet;

/**
 * 从 Termux-x11 复制而来。处理 activity 的一般按键事件（如输入法）。
 */
class KeyEventSender {
    static KeyEventSender instance = new KeyEventSender();
    public boolean preferScancodes = false;
    /** Set of pressed keys for which we've sent TextEvent. */
    private final TreeSet<Integer> mPressedTextKeys = new TreeSet<>();
    private final TreeSet<Integer> mPressedKeys = new TreeSet<>();

    /**
     * Converts the {@link KeyEvent} into low-level events and sends them to the host as either
     * key-events or text-events. This contains some logic for handling some special keys, and
     * avoids sending a key-up event for a key that was previously injected as a text-event.
     */
    boolean sendKeyEvent(KeyEvent e, LorieView lorieView) {
        int keyCode = e.getKeyCode();
        // 返回键要显示菜单
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            return false;
        }
        boolean pressed = e.getAction() == KeyEvent.ACTION_DOWN;

        if ((e.getFlags() & KeyEvent.FLAG_CANCELED) == KeyEvent.FLAG_CANCELED) {
            android.util.Log.d("KeyEvent", "We've got key event with FLAG_CANCELED, it will not be consumed. Details: " + e);
            return true;
        }

        // Events received from software keyboards generate TextEvent in two
        // cases:
        //   1. This is an ACTION_MULTIPLE event.
        //   2. Ctrl, Alt and Meta are not pressed.
        // This ensures that on-screen keyboard always injects input that
        // correspond to what user sees on the screen, while physical keyboard
        // acts as if it is connected to the remote host.
        if (e.getAction() == ACTION_MULTIPLE) {
            if (e.getCharacters() != null)
                lorieView.sendTextEvent(e.getCharacters().getBytes(UTF_8));
            else if (e.getUnicodeChar() != 0)
                lorieView.sendTextEvent(String.valueOf((char) e.getUnicodeChar()).getBytes(UTF_8));
            return true;
        }

        boolean no_modifiers = (!e.isAltPressed() && !e.isCtrlPressed() && !e.isMetaPressed())
                || ((e.getMetaState() & META_ALT_RIGHT_ON) != 0 && (e.getCharacters() != null || e.getUnicodeChar() != 0)); // For layouts with AltGr
        // For Enter getUnicodeChar() returns 10 (line feed), but we still
        // want to send it as KeyEvent.
        char unicode = keyCode != KEYCODE_ENTER ? (char) e.getUnicodeChar() : 0;
        int scancode = (preferScancodes || !no_modifiers) ? e.getScanCode() : 0;

        if (!preferScancodes) {
            if (pressed && unicode != 0 && no_modifiers) {
                mPressedTextKeys.add(keyCode);
                if ((e.getMetaState() & META_ALT_RIGHT_ON) != 0)
                    lorieView.sendKeyEvent(0, KEYCODE_ALT_RIGHT, false); // For layouts with AltGr

                lorieView.sendTextEvent(String.valueOf(unicode).getBytes(UTF_8));

                if ((e.getMetaState() & META_ALT_RIGHT_ON) != 0)
                    lorieView.sendKeyEvent(0, KEYCODE_ALT_RIGHT, true); // For layouts with AltGr
                return true;
            }

            if (!pressed && mPressedTextKeys.contains(keyCode)) {
                mPressedTextKeys.remove(keyCode);
                return true;
            }
        }

        // KEYCODE_AT, KEYCODE_POUND, KEYCODE_STAR and KEYCODE_PLUS are
        // deprecated, but they still need to be here for older devices and
        // third-party keyboards that may still generate these events. See
        // https://source.android.com/devices/input/keyboard-devices.html#legacy-unsupported-keys
        char[][] chars = {
                {KEYCODE_AT, '@', KEYCODE_2},
                {KEYCODE_POUND, '#', KEYCODE_3},
                {KEYCODE_STAR, '*', KEYCODE_8},
                {KEYCODE_PLUS, '+', KEYCODE_EQUALS}
        };

        for (char[] i : chars) {
            if (e.getKeyCode() != i[0])
                continue;

            if ((e.getCharacters() != null && String.valueOf(i[1]).contentEquals(e.getCharacters()))
                    || e.getUnicodeChar() == i[1]) {
                lorieView.sendKeyEvent(0, KEYCODE_SHIFT_LEFT, pressed);
                lorieView.sendKeyEvent(0, i[2], pressed);
                return true;
            }
        }

        // Ignoring Android's autorepeat.
        // But some weird IMEs (or firmwares) send first event with repeatCount=1 (not 0)
        // Probably related to preceding event with FLAG_CANCELLED flag
        if (e.getRepeatCount() > 0 && mPressedKeys.contains(keyCode))
            return true;

        if (pressed)
            mPressedKeys.add(keyCode);
        else
            mPressedKeys.remove(keyCode);

//        if (keyCode == KEYCODE_ESCAPE && !pressed && e.hasNoModifiers())
//            MainActivity.setCapturingEnabled(false);

        // We try to send all other key codes to the host directly.
        return lorieView.sendKeyEvent(scancode, keyCode, pressed);
    }
}
