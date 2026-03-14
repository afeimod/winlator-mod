package com.ewt45.winlator;
import android.view.*;
import com.winlator.xserver.*;
import java.util.concurrent.atomic.*;

public class E02_KeyInput {
    private static final String TAG = "E2_KeyInput";
    public static final XKeycode[] stubKeyCode = {
        XKeycode.KEY_CUSTOM_1, XKeycode.KEY_CUSTOM_2, XKeycode.KEY_CUSTOM_3, 
        XKeycode.KEY_CUSTOM_4, XKeycode.KEY_CUSTOM_5, XKeycode.KEY_CUSTOM_6,
        XKeycode.KEY_CUSTOM_7, XKeycode.KEY_CUSTOM_8, XKeycode.KEY_CUSTOM_9,
        XKeycode.KEY_CUSTOM_10, XKeycode.KEY_CUSTOM_11, XKeycode.KEY_CUSTOM_12,
        XKeycode.KEY_CUSTOM_13, XKeycode.KEY_CUSTOM_14, XKeycode.KEY_CUSTOM_15,
        XKeycode.KEY_CUSTOM_16, XKeycode.KEY_CUSTOM_17
    };

    // 使用 AtomicInteger 替代普通的 int
    private static final AtomicInteger currIndex = new AtomicInteger(0);

    public static boolean handleAndroidKeyEvent(XServer xServer, KeyEvent event) {
    boolean handled = false;
    if (event.getAction() == KeyEvent.ACTION_MULTIPLE) {
        String characters = event.getCharacters();
        
        // 添加空值检查
        if (characters == null) {
            return false;
        }
        
        for (int i = 0; i < characters.codePointCount(0, characters.length()); i++) {
            // 原子性地获取并增加索引
            int index = currIndex.getAndUpdate(curr -> (curr + 1) % stubKeyCode.length);
            int keycode = stubKeyCode[index].id;
            int keySym = characters.codePointAt(characters.offsetByCodePoints(0, i));

            if (keySym > 0xff) keySym = keySym | 0x1000000;

            xServer.injectKeyPress(stubKeyCode[index], keySym);
            sleep();
            xServer.injectKeyRelease(stubKeyCode[index]);                
            sleep();
            handled = true;
        }
    }
    return handled;
}


    private static void sleep() {
        try {
            Thread.sleep(50);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
