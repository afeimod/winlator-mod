package com.winlator.win32;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.winlator.core.FileUtils;
import com.winlator.core.ImageUtils;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Collections;

public class MSIcon {
    private static class IconDirEntry {
        private int width;
        private int height;
        private byte numberOfColors;
        private byte reserved;
        private short colorPlanes;
        private short bitCount;
        private int imageSize;
        private int imageOffset;
    }

    public static Bitmap decodeByteArray(byte[] bytes, int offset, int length) {
        ByteBuffer data = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        data.position(offset);

        if (ImageUtils.isPNGData(data)) {
            return BitmapFactory.decodeByteArray(bytes, offset, length);
        }
        else {
            int bitmapOffset = data.getInt();
            int bmpWidth = data.getInt();
            int bmpHeight = data.getInt();
            short colorPlanes = data.getShort();
            short bitCount = data.getShort();

            data.position(offset + bitmapOffset);
            return MSBitmap.decodeBuffer(bmpWidth, bmpWidth, bitCount, data);
        }
    }

    public static Bitmap decodeFile(File icoFile) {
        if (!icoFile.isFile()) return null;
        byte[] bytes = FileUtils.read(icoFile);
        if (bytes == null) return null;

        ByteBuffer data = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);

        short reserved = data.getShort();
        short imageType = data.getShort();
        short numberOfImages = data.getShort();

        if (reserved != 0 || imageType != 1 || (numberOfImages <= 0 || numberOfImages >= 32)) return null;

        ArrayList<IconDirEntry> entries = new ArrayList<>();

        for (byte i = 0; i < numberOfImages; i++) {
            IconDirEntry entry = new IconDirEntry();
            entry.width = Byte.toUnsignedInt(data.get());
            entry.height = Byte.toUnsignedInt(data.get());
            entry.numberOfColors = data.get();
            entry.reserved = data.get();
            entry.colorPlanes = data.getShort();
            entry.bitCount = data.getShort();
            entry.imageSize = data.getInt();
            entry.imageOffset = data.getInt();
            entries.add(entry);
        }

        Collections.sort(entries, (a, b) -> {
            int value = Short.compare(b.bitCount, a.bitCount);
            if (value != 0) return value;
            return Integer.compare(b.width, a.width);
        });

        IconDirEntry firstEntry = entries.get(0);
        return decodeByteArray(data.array(), firstEntry.imageOffset, firstEntry.imageSize);
    }
}
