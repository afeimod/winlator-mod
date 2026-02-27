#ifndef WINLATOR_IMAGE_UTILS_H
#define WINLATOR_IMAGE_UTILS_H

#include "arrays.h"
#include "file_utils.h"

static inline void writeBMPImageFile(char* filename, int width, int height, uint8_t channels, void* imageData) {
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

    ArrayBuffer data = {0};
    ENSURE_ARRAY_CAPACITY(dataOffset + imageSize, data.capacity, data.buffer, 1);

    ArrayBuffer_putShort(&data, 0x4d42); // "BM"
    ArrayBuffer_putInt(&data, dataOffset + imageSize);
    ArrayBuffer_putInt(&data, 0);
    ArrayBuffer_putInt(&data, dataOffset);

    ArrayBuffer_putInt(&data, infoHeaderSize);
    ArrayBuffer_putInt(&data, width);
    ArrayBuffer_putInt(&data, height);
    ArrayBuffer_putShort(&data, planes);
    ArrayBuffer_putShort(&data, bitCount);
    ArrayBuffer_putInt(&data, compression);
    ArrayBuffer_putInt(&data, imageSize);
    ArrayBuffer_putInt(&data, hr);
    ArrayBuffer_putInt(&data, vr);
    ArrayBuffer_putInt(&data, colorsUsed);
    ArrayBuffer_putInt(&data, colorsImportant);

    int rowBytes = 3 * width + extraBytes;
    uint8_t* pixels = imageData;
    for (int y = height - 1, i = 0, j; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            j = dataOffset + y * rowBytes + x * 3;
            if (channels >= 3) {
                data.buffer[j+0] = pixels[i+2];
                data.buffer[j+1] = pixels[i+1];
                data.buffer[j+2] = pixels[i+0];
            }
            else {
                data.buffer[j+0] = 0;
                data.buffer[j+1] = 0;
                data.buffer[j+2] = pixels[i+0];
            }
            i += channels;
        }

        if (extraBytes > 0) {
            int fillOffset = dataOffset + y * rowBytes + width * 3;
            for (j = fillOffset; j < fillOffset + extraBytes; j++) data.buffer[j] = 255;
        }
    }

    filePutContents(filename, data.buffer, data.capacity);
}

static inline void swapPixelsRedBlue(char* pixelData, int bytesPerPixel, int size) {
    if (bytesPerPixel == 0) bytesPerPixel = 4;
    for (int i = 0; i < size; i += bytesPerPixel) {
        char tmp = pixelData[i+0];
        pixelData[i+0] = pixelData[i+2];
        pixelData[i+2] = tmp;
    }
}

#endif