// based on SwiftShader (https://github.com/google/swiftshader/blob/master/src/Device/BC_Decoder.cpp)

#ifndef BC_DECODER_H
#define BC_DECODER_H

#define BC_BLOCK 4u

typedef struct BCColor {
    uint16_t c0;
    uint16_t c1;
    uint32_t idx;
} BCColor;

typedef struct BCTask {
    const uint8_t* src;
    uint8_t* dst;
    int width;
    int height;
    int startY;
    int endY;
    int bcN;
    bool isAlpha;
    bool isSigned;
} BCTask;

static inline uint32_t BCDecoder_getColorIdx(const BCColor* color, int i) {
    int offset = i << 1;
    return (color->idx & (0x3 << offset)) >> offset;
}

static inline uint8_t BCDecoder_getChannelIdx(uint64_t data, int i) {
    int offset = i * 3 + 16;
    return (uint8_t)((data & (0x7ull << offset)) >> offset);
}

static inline uint8_t BCDecoder_getAlpha(uint64_t data, int i) {
    int offset = i << 2;
    int alpha = (data & (0xFull << offset)) >> offset;
    return (uint8_t)(alpha | (alpha << 4));
}

static inline int BCDecoder_pack8888(int* c) {
    return ((c[2] & 0xFF) << 16) | ((c[1] & 0xFF) << 8) | (c[0] & 0xFF) | c[3];
}

static inline void BCDecoder_extract565(int* c, uint32_t c565) {
    c[0] = ((c565 & 0x0000001F) << 3) | ((c565 & 0x0000001C) >> 2);
    c[1] = ((c565 & 0x000007E0) >> 3) | ((c565 & 0x00000600) >> 9);
    c[2] = ((c565 & 0x0000F800) >> 8) | ((c565 & 0x0000E000) >> 13);
}

static inline void BCDecoder_decodeAlpha(uint64_t data, uint8_t* dst, int x, int y, int width, int height, int stride) {
    dst += 3;
    for (int j = 0; j < BC_BLOCK && (y + j) < height; j++, dst += stride) {
        uint8_t* dstRow = dst;
        for (int i = 0; i < BC_BLOCK && (x + i) < width; i++, dstRow += 4) {
            *dstRow = BCDecoder_getAlpha(data, j * BC_BLOCK + i);
        }
    }
}

static inline void BCDecoder_decodeChannel(uint64_t data, uint8_t* dst, int x, int y, int width, int height, int stride, int channel, bool isSigned) {
    int c[8] = {0};

    if (isSigned) {
        c[0] = (signed char)(data & 0xFF);
        c[1] = (signed char)((data & 0xFF00) >> 8);
    }
    else {
        c[0] = (uint8_t)(data & 0xFF);
        c[1] = (uint8_t)((data & 0xFF00) >> 8);
    }

    if (c[0] > c[1]) {
        for (int i = 2; i < 8; ++i) {
            c[i] = ((8 - i) * c[0] + (i - 1) * c[1]) / 7;
        }
    }
    else {
        for (int i = 2; i < 6; i++) {
            c[i] = ((6 - i) * c[0] + (i - 1) * c[1]) / 5;
        }

        c[6] = isSigned ? -128 : 0;
        c[7] = isSigned ?  127 : 255;
    }

    for (int i, j = 0; j < BC_BLOCK && (y + j) < height; j++) {
        for (i = 0; i < BC_BLOCK && (x + i) < width; i++) {
            dst[channel + (i * 4) + (j * stride)] = (uint8_t)c[BCDecoder_getChannelIdx(data, (j * BC_BLOCK) + i)];
        }
    }
}

static inline void BCDecoder_decodeColor(const BCColor* color, uint8_t* dst, int x, int y, int width, int height, int stride, bool hasAlphaChannel, bool hasSeparateAlpha) {
    int c[4][4] = {0};
    c[0][3] = 0xFF000000;
    c[1][3] = 0xFF000000;
    c[2][3] = 0xFF000000;
    c[3][3] = 0xFF000000;

    BCDecoder_extract565(c[0], color->c0);
    BCDecoder_extract565(c[1], color->c1);

    if (hasSeparateAlpha || (color->c0 > color->c1)) {
        for (int i = 0; i < 4; i++) {
            c[2][i] = ((c[0][i] * 2) + c[1][i]) / 3;
            c[3][i] = ((c[1][i] * 2) + c[0][i]) / 3;
        }
    }
    else {
        for (int i = 0; i < 4; i++) c[2][i] = (c[0][i] + c[1][i]) >> 1;
        if (hasAlphaChannel) c[3][3] = 0;
    }

    for (int i, j = 0, dstOffset, idxOffset; j < BC_BLOCK && (y + j) < height; j++) {
        dstOffset = j * stride;
        idxOffset = j * BC_BLOCK;
        for (i = 0; i < BC_BLOCK && (x + i) < width; i++, idxOffset++, dstOffset += 4) {
            *(uint32_t*)(dst + dstOffset) = BCDecoder_pack8888(c[BCDecoder_getColorIdx(color, idxOffset)]);
        }
    }
}

static inline void BCDecoder_decodeThread(void* param) {
    BCTask* task = param;
    const int stride = task->width * 4;
    const int dx = 4 * BC_BLOCK;
    const int dy = stride * BC_BLOCK;

    switch (task->bcN) {
        case 1: {
            const BCColor* color = (const BCColor*)task->src;
            for (int x, y = task->startY; y < task->endY; y += BC_BLOCK, task->dst += dy) {
                uint8_t* dstRow = task->dst;
                for (x = 0; x < task->width; x += BC_BLOCK, ++color, dstRow += dx) {
                    BCDecoder_decodeColor(color, dstRow, x, y, task->width, task->height, stride, task->isAlpha, false);
                }
            }
            break;
        }
        case 2: {
            const uint64_t* alpha = (const uint64_t*)(task->src);
            const BCColor* color = (const BCColor*)(task->src + 8);
            for (int x, y = task->startY; y < task->endY; y += BC_BLOCK, task->dst += dy) {
                uint8_t* dstRow = task->dst;
                for (x = 0; x < task->width; x += BC_BLOCK, alpha += 2, color += 2, dstRow += dx) {
                    BCDecoder_decodeColor(color, dstRow, x, y, task->width, task->height, stride, task->isAlpha, true);
                    BCDecoder_decodeAlpha(*alpha, dstRow, x, y, task->width, task->height, stride);
                }
            }
            break;
        }
        case 3: {
            const uint64_t* alpha = (const uint64_t*)(task->src);
            const BCColor* color = (const BCColor*)(task->src + 8);
            for (int x, y = task->startY; y < task->endY; y += BC_BLOCK, task->dst += dy) {
                uint8_t *dstRow = task->dst;
                for (x = 0; x < task->width; x += BC_BLOCK, alpha += 2, color += 2, dstRow += dx) {
                    BCDecoder_decodeColor(color, dstRow, x, y, task->width, task->height, stride, task->isAlpha, true);
                    BCDecoder_decodeChannel(*alpha, dstRow, x, y, task->width, task->height, stride, 3, task->isSigned);
                }
            }
            break;
        }
        case 4: {
            const uint64_t* red = (const uint64_t*)(task->src);
            for (int x, y = task->startY; y < task->endY; y += BC_BLOCK, task->dst += dy) {
                uint8_t *dstRow = task->dst;
                for (x = 0; x < task->width; x += BC_BLOCK, ++red, dstRow += dx) {
                    BCDecoder_decodeChannel(*red, dstRow, x, y, task->width, task->height, stride, 0, task->isSigned);
                }
            }
            break;
        }
        case 5: {
            const uint64_t* red = (const uint64_t*)(task->src);
            const uint64_t* green = (const uint64_t*)(task->src + 8);
            for (int y = task->startY; y < task->endY; y += BC_BLOCK, task->dst += dy) {
                uint8_t* dstRow = task->dst;
                for (int x = 0; x < task->width; x += BC_BLOCK, red += 2, green += 2, dstRow += dx) {
                    BCDecoder_decodeChannel(*red, dstRow, x, y, task->width, task->height, stride, 0, task->isSigned);
                    BCDecoder_decodeChannel(*green, dstRow, x, y, task->width, task->height, stride, 1, task->isSigned);
                }
            }
            break;
        }
    }
}

static inline void BCDecoder_decode(const uint8_t* src, uint8_t* dst, int width, int height, int bcN, bool isNoAlphaU, ThreadPool* threadPool) {
    const int stride = width * 4;
    const int dy = stride * BC_BLOCK;
    const bool isAlpha = (bcN == 1) && !isNoAlphaU;
    const bool isSigned = (bcN == 4 || bcN == 5) && !isNoAlphaU;
    const int blockSize = (bcN == 1 || bcN == 4) ? 8 : 16;

    int numTasks = height >= 512 ? 4 : (height >= 128 ? 2 : 1);
    BCTask tasks[numTasks];

    int divHeight = height / numTasks;
    for (int i = 0, y; i < numTasks; i++) {
        BCTask* task = &tasks[i];
        task->bcN = bcN;
        task->width = width;
        task->height = height;
        task->startY = i * divHeight;
        task->endY = task->startY + divHeight;
        y = i * (divHeight / BC_BLOCK);
        task->src = src + (y * (width / BC_BLOCK) * blockSize);
        task->dst = dst + (y * dy);
        task->isAlpha = isAlpha;
        task->isSigned = isSigned;

        if (numTasks > 1) {
            ThreadPool_run(threadPool, BCDecoder_decodeThread, task);
        }
        else BCDecoder_decodeThread(task);
    }

    if (numTasks > 1) ThreadPool_wait(threadPool);
}

#endif