package com.winlator.win32;

import android.graphics.Bitmap;
import android.graphics.Color;

import com.winlator.core.FileUtils;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public abstract class MSBitmap {
    public static Bitmap decodeBuffer(int width, int height, int bitCount, ByteBuffer data) {
        if (width == 0 || height == 0) return null;
        byte r, g, b, a;
        int i, j, x, y, line;

        boolean invertY = true;
        if (height < 0) {
            height *= -1;
            invertY = false;
        }

        ByteBuffer pixels = ByteBuffer.allocate(width * height * 4);

        if (bitCount == 32) {
            for (y = height - 1, i = data.position(); y >= 0; y--) {
                line = invertY ? y : height - 1 - y;

                for (x = 0; x < width; x++) {
                    b = data.get(i++);
                    g = data.get(i++);
                    r = data.get(i++);
                    a = data.get(i++);

                    j = line * width * 4 + x * 4;
                    pixels.put(j+2, b);
                    pixels.put(j+1, g);
                    pixels.put(j+0, r);
                    pixels.put(j+3, a);
                }

                i += width % 4;
            }
        }
        else if (bitCount == 24) {
            for (y = height - 1, i = data.position(); y >= 0; y--) {
                line = invertY ? y : height - 1 - y;

                for (x = 0; x < width; x++) {
                    b = data.get(i++);
                    g = data.get(i++);
                    r = data.get(i++);

                    j = line * width * 4 + x * 4;
                    pixels.put(j+2, b);
                    pixels.put(j+1, g);
                    pixels.put(j+0, r);
                    pixels.put(j+3, (byte)255);
                }

                i += width % 4;
            }
        }
        else if (bitCount <= 8) {
            final int colorTableOffset = data.position();
            final int colorTableSize = (int)(Math.pow(2, bitCount) * 4);
            int colorIndex;

            for (y = height - 1, i = data.position() + colorTableSize; y >= 0; y--) {
                line = invertY ? y : height - 1 - y;

                for (x = 0; x < width; x++) {
                    colorIndex = Byte.toUnsignedInt(data.get(i++)) * 4;

                    b = data.get(colorTableOffset + colorIndex + 0);
                    g = data.get(colorTableOffset + colorIndex + 1);
                    r = data.get(colorTableOffset + colorIndex + 2);

                    j = line * width * 4 + x * 4;
                    pixels.put(j+2, b);
                    pixels.put(j+1, g);
                    pixels.put(j+0, r);
                    pixels.put(j+3, (byte)255);
                }

                i += width % 4;
            }
        }

        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(pixels);
        return bitmap;
    }

    public static Bitmap decodeFile(File bmpFile) {
        if (!bmpFile.isFile()) return null;
        byte[] bytes = FileUtils.read(bmpFile);
        if (bytes == null) return null;

        ByteBuffer data = ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN);
        if (data.getShort() != 0x4d42) return null;

        int fileSize = data.getInt();
        if (fileSize > bmpFile.length()) return null;

        data.getInt();
        int dataOffset = data.getInt();
        int infoHeaderSize = data.getInt();
        int width = data.getInt();
        int height = data.getInt();
        short planes = data.getShort();
        short bitCount = data.getShort();
        int compression = data.getInt();
        int imageSize = data.getInt();
        int hr = data.getInt();
        int vr = data.getInt();
        int colorsUsed = data.getInt();
        int colorsImportant = data.getInt();

        return decodeBuffer(width, height, bitCount, data);
    }

    public static boolean create(Bitmap bitmap, File outputFile) {
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();

        int[] pixels = new int[width * height];
        bitmap.getPixels(pixels, 0, width, 0, 0, width, height);

        int extraBytes = width % 4;
        int imageSize = height * (3 * width + extraBytes);
        int infoHeaderSize = 40;
        int dataOffset = 54;
        int bitCount = 24;
        int planes = 1;
        int compression = 0;
        int hr = 0;
        int vr = 0;
        int colorsUsed = 0;
        int colorsImportant = 0;
        
        ByteBuffer buffer = ByteBuffer.allocate(dataOffset + imageSize).order(ByteOrder.LITTLE_ENDIAN);

        buffer.putShort((short)0x4d42); // "BM"
        buffer.putInt(dataOffset + imageSize);
        buffer.putInt(0);
        buffer.putInt(dataOffset);

        buffer.putInt(infoHeaderSize);
        buffer.putInt(width);
        buffer.putInt(height);
        buffer.putShort((short)planes);
        buffer.putShort((short)bitCount);
        buffer.putInt(compression);
        buffer.putInt(imageSize);
        buffer.putInt(hr);
        buffer.putInt(vr);
        buffer.putInt(colorsUsed);
        buffer.putInt(colorsImportant);

        int rowBytes = 3 * width + extraBytes;
        for (int y = height - 1, i = 0, j; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                j = dataOffset + y * rowBytes + x * 3;
                int pixel = pixels[i++];
                buffer.put(j+0, (byte)Color.blue(pixel));
                buffer.put(j+1, (byte)Color.green(pixel));
                buffer.put(j+2, (byte)Color.red(pixel));
            }

            if (extraBytes > 0) {
                int fillOffset = dataOffset + y * rowBytes + width * 3;
                for (j = fillOffset; j < fillOffset + extraBytes; j++) buffer.put(j, (byte)255);
            }
        }

        try (FileOutputStream fos = new FileOutputStream(outputFile)) {
            fos.write(buffer.array());
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }
}
