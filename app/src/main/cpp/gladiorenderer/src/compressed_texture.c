#define STB_DXT_IMPLEMENTATION
#include "stb_dxt.h"

#include "compressed_texture.h"
#include "bc_decoder.h"

void compressTexImage2D(uint32_t format, int width, int height, void* imageData, void* compressedData) {
    static const uint8_t offsets[16][2] = {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {0, 1}, {1, 1}, {2, 1}, {3, 1}, {0, 2}, {1, 2}, {2, 2}, {3, 2}, {0, 3}, {1, 3}, {2, 3}, {3, 3}};
    int blockSize = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16;
    int alpha = (format != GL_COMPRESSED_RGB_S3TC_DXT1_EXT && blockSize == 16) ? 1 : 0;

    GLuint* src = imageData;
    GLuint block[16];
    for (int y = 0, x, i; y < height; y += 4) {
        for (x = 0; x < width; x += 4) {
            for (i = 0; i < 16; i++) {
                block[i] = (x+offsets[i][0]) < width && (y+offsets[i][1]) < height ? src[x+offsets[i][0]+(y+offsets[i][1])*width] : 0;
            }
            stb_compress_dxt_block((uint8_t*)compressedData, (const uint8_t*)block, alpha, STB_DXT_NORMAL);
            compressedData += blockSize;
        }
    }
}

void* decompressTexImage2D(uint32_t format, int width, int height, void* imageData, ThreadPool* threadPool) {
    int bcN = 0;
    bool isNoAlphaU = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

    switch (format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            bcN = 1;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            bcN = 2;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            bcN = 3;
            break;
    }

    void* decompressedData = malloc(width * height * 4);
    BCDecoder_decode(imageData, decompressedData, width, height, bcN, isNoAlphaU, threadPool);
    return decompressedData;
}