#ifndef GLADIO_COMPRESSED_TEXTURE_H
#define GLADIO_COMPRESSED_TEXTURE_H

#include "gladio.h"
#include "thread_pool.h"

static inline int getCompressedImageSize(uint32_t format, int width, int height, int level) {
    width = TEXLEVEL(width >> level, 2) << 2;
    height = TEXLEVEL(height >> level, 2) << 2;
    return format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? (width * height) / 2 : width * height;
}

extern void compressTexImage2D(uint32_t format, int width, int height, void* imageData, void* compressedData);
extern void* decompressTexImage2D(uint32_t format, int width, int height, void* imageData, ThreadPool* threadPool);

#endif
