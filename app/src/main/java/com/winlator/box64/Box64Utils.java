package com.winlator.box64;

import android.content.Context;

import com.winlator.core.ArrayUtils;
import com.winlator.core.StreamUtils;
import com.winlator.xenvironment.RootFS;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public abstract class Box64Utils {
    public static String extractBinVersion(Context context) {
        File binFile = new File(RootFS.find(context).getRootDir(), "/usr/local/bin/box64");
        try (BufferedInputStream inStream = new BufferedInputStream(new FileInputStream(binFile), StreamUtils.BUFFER_SIZE)) {
            int bytesRead;
            byte[] buffer = new byte[4096];
            final byte[] str = {'B','o','x','6','4',' ','a','r','m','6','4',' ','v'};
            while ((bytesRead = inStream.read(buffer)) != -1) {
                int index = ArrayUtils.indexOf(buffer, 0, bytesRead, str);
                if (index != ArrayUtils.INDEX_NOT_FOUND) {
                    int start = index + str.length;
                    int end = ArrayUtils.indexOf(buffer, start, bytesRead, (byte)' ');
                    return end != ArrayUtils.INDEX_NOT_FOUND ? new String(buffer, start, end - start) : "";
                }
            }
        }
        catch (IOException e) {}
        return "";
    }
}
