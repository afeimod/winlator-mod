#ifndef GLADIO_GL_FORMATS_H
#define GLADIO_GL_FORMATS_H

#include "gladio.h"

typedef struct GLFormatInfo {
    GLenum internalformat;
    GLenum format;
    GLenum componentType;
    union {
        uint8_t redSize;
        uint8_t depthSize;
    };
    union {
        uint8_t greenSize;
        uint8_t stencilSize;
    };
    uint8_t blueSize;
    uint8_t alphaSize;
    uint8_t sharedSize;
    bool colorRenderable;
    bool textureFilterable;
} GLFormatInfo;

extern void GLFormats_queryInternalformat(GLenum target, GLenum internalformat, GLenum pname, int count, int* params);

#endif
