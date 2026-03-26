#ifndef GLADIO_TEXTURE_UTILS_H
#define GLADIO_TEXTURE_UTILS_H

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#endif

#include "gl_enums.h"
#include "string_utils.h"

#define GL_LUMINANCE_LEGACY 1
#define GL_LUMINANCE_ALPHA_LEGACY 2
#define GL_RGB_LEGACY 3
#define GL_RGBA_LEGACY 4

#define TEXLEVEL(x, y) MAX(1, (x) >> (y))

static inline int getTexImageFormatChannels(uint32_t format) {
    switch (format) {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_GREEN:
        case GL_BLUE:
        case GL_LUMINANCE:
        case GL_ALPHA:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_ALPHA8:
            return 1;
        case GL_RG:
        case GL_RG_INTEGER:
        case GL_LUMINANCE_ALPHA:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
            return 2;
        case GL_RGB:
        case GL_BGR:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_SRGB8:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGB565:
            return 3;
        case GL_RGBA:
        case GL_BGRA:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            return 4;
        default:
            return 0;
    }
}

static inline int computeTexImageDataSize(uint32_t format, uint32_t type, uint32_t width, uint32_t height, uint32_t depth) {
    int channels = 0;
    int bytesPerPixel = 0;

    switch (format) {
        case GL_RED:
        case GL_RED_INTEGER:
        case GL_GREEN:
        case GL_BLUE:
        case GL_LUMINANCE:
        case GL_ALPHA:
            channels = 1;
            break;
        case GL_RG:
        case GL_RG_INTEGER:
        case GL_LUMINANCE_ALPHA:
            channels = 2;
            break;
        case GL_RGB:
        case GL_BGR:
        case GL_RGB_INTEGER:
        case GL_BGR_INTEGER:
            channels = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
        case GL_RGBA_INTEGER:
        case GL_BGRA_INTEGER:
            channels = 4;
            break;
    }

    switch (type) {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            bytesPerPixel = 1;
            break;
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
            channels = 1;
            bytesPerPixel = 1;
            break;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
            bytesPerPixel = 2;
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            channels = 1;
            bytesPerPixel = 2;
            break;
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            bytesPerPixel = 4;
            break;
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_INT_10_10_10_2:
            channels = 1;
            bytesPerPixel = 4;
            break;
    }

    return width * height * depth * channels * bytesPerPixel;
}

#ifdef __ANDROID__
static inline bool isCompressedFormat(GLenum format) {
    switch (format) {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return true;
        default:
            return false;
    }
}

static inline void convertTexImageFormat(uint32_t target, int* internalformat, uint32_t* format, uint32_t* type, void** imageData, int imageSize) {
    int none = GL_NONE;
    if (!internalformat) internalformat = &none;

#if IS_DEBUG_ENABLED(DEBUG_MODE_TEXTURE_FORMAT)
    static SparseArray debugFormats = {0};
    const int key[] = {*internalformat, *format, *type};
    uint32_t hash = fnv1aHash32(key, sizeof(key));
    if (SparseArray_indexOfKey(&debugFormats, hash) < 0) {
        SparseArray_put(&debugFormats, hash, NULL);
        println("gladio:convertTexImageFormat: %s %s %s %s", glEnumToString(target), glEnumToString(*internalformat), glEnumToString(*format), glEnumToString(*type));
    }
#endif

    if (*internalformat == GL_LUMINANCE_LEGACY ||
        *internalformat == GL_LUMINANCE_ALPHA_LEGACY ||
        *internalformat == GL_RGB_LEGACY ||
        *internalformat == GL_RGBA_LEGACY) {

        if (*internalformat == GL_LUMINANCE_LEGACY || *internalformat == GL_RGB_LEGACY) {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
        }

        switch (*format) {
            case GL_RGBA:
                *internalformat = GL_RGBA8;
                break;
            case GL_RGB:
                *internalformat = GL_RGB8;
                break;
            case GL_LUMINANCE_ALPHA:
                *internalformat = GL_RG8;
                break;
        }
    }

    if (isCompressedFormat(*internalformat) && imageSize == 0) {
        *internalformat = GL_BGRA;
        *format = GL_BGRA;
    }
    if (*type == GL_UNSIGNED_INT_8_8_8_8 || *type == GL_UNSIGNED_INT_8_8_8_8_REV) {
        *type = GL_UNSIGNED_BYTE;
    }
    else if (*type == GL_BYTE) *type = GL_UNSIGNED_BYTE;

    switch (*format) {
        case GL_ALPHA: {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_RED);
            *internalformat = GL_R8;
            *format = GL_RED;
            break;
        }
        case GL_LUMINANCE: {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
            *internalformat = GL_R8;
            *format = GL_RED;
            break;
        }
        case GL_INTENSITY:
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_R, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_G, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_B, GL_RED);
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_RED);
            *internalformat = GL_R8;
            *format = GL_RED;
            break;
        case GL_RED:
            if (*internalformat == GL_RED) *internalformat = GL_R8;
            break;
        case GL_BGR:
            *internalformat = GL_RGB8;
            *format = GL_RGB;
            break;
        case GL_RGB:
            if (*type == GL_UNSIGNED_SHORT_5_6_5) *internalformat = GL_RGB565;
            break;
        case GL_BGRA:
            if (*type == GL_UNSIGNED_SHORT_1_5_5_5_REV) {
                *type = GL_UNSIGNED_SHORT_5_5_5_1;
                *internalformat = GL_RGB5_A1;
                *format = GL_RGBA;

                if (imageSize > 0) {
                    GLushort* pixels = *imageData;
                    for (int i = 0; i < imageSize / sizeof(GLushort); i++) {
                        GLushort pixel = pixels[i];
                        GLushort r = pixel & 0x1f;
                        GLushort g = (pixel >> 5) & 0x1f;
                        GLushort b = (pixel >> 10) & 0x1f;
                        GLushort a = (pixel >> 15) & 0x01;
                        pixels[i] = (a & 0x01) | ((r & 0x1f) << 1) | ((g & 0x1f) << 6) | ((b & 0x1f) << 11);
                    }
                }
            }
            else if (*type == GL_UNSIGNED_SHORT_4_4_4_4_REV) {
                *type = GL_UNSIGNED_SHORT_4_4_4_4;
                *internalformat = GL_RGBA4;
                *format = GL_RGBA;

                if (imageSize > 0) {
                    GLushort* pixels = *imageData;
                    for (int i = 0; i < imageSize / sizeof(GLushort); i++) {
                        GLushort pixel = pixels[i];
                        GLushort r = pixel & 0x0f;
                        GLushort g = (pixel >> 4) & 0x0f;
                        GLushort b = (pixel >> 8) & 0x0f;
                        GLushort a = (pixel >> 12) & 0x0f;
                        pixels[i] = ((b & 0x0f) << 12) | ((g & 0x0f) << 8) | ((r & 0x0f) << 4) | (a & 0x0f);
                    }
                }
            }
            break;
        case GL_RGBA:
            if (*type == GL_UNSIGNED_SHORT) {
                *internalformat = GL_RGBA16UI;
                *format = GL_RGBA_INTEGER;
            }
            break;
        case GL_DEPTH_COMPONENT:
            if (*internalformat == GL_DEPTH_COMPONENT) {
                *internalformat = GL_DEPTH_COMPONENT24;
                *type = GL_UNSIGNED_INT;
            }
            break;
    }

    int internalformatChannels = getTexImageFormatChannels(*internalformat);
    int formatChannels = getTexImageFormatChannels(*format);
    if (internalformatChannels != formatChannels) *internalformat = GL_NONE;

    if (*internalformat == GL_NONE) {
        switch (*format) {
            case GL_RED:
            case GL_GREEN:
            case GL_BLUE:
            case GL_ALPHA:
            case GL_LUMINANCE:
            case GL_INTENSITY:
                *internalformat = GL_R8;
                *format = GL_RED;
                break;
            case GL_LUMINANCE_ALPHA:
                *internalformat = GL_RG8;
                *format = GL_RG;
                break;
            case GL_RGB:
                *internalformat = GL_RGB8;
                break;
            case GL_BGRA:
                *internalformat = GL_BGRA;
            case GL_RGBA:
                *internalformat = GL_RGBA8;
                break;
        }

        if (internalformatChannels == 3 && formatChannels == 4) {
            glTexParameteri(target, GL_TEXTURE_SWIZZLE_A, GL_ONE);
        }
        else if (internalformatChannels == 1 && formatChannels > 1) {
            *internalformat = GL_R8;
            *format = GL_RED;

            if (imageSize > 0 && *type == GL_UNSIGNED_BYTE) {
                imageSize /= 4;
                char* alphaPixels = malloc(imageSize);
                GLubyte* pixels = *imageData;
                for (int i = 0; i < imageSize; i++) alphaPixels[i] = pixels[i*4];
                memcpy(*imageData, alphaPixels, imageSize);
                free(alphaPixels);
            }
        }
    }

    if (*format == GL_BGRA) *internalformat = GL_BGRA;
}
#endif

static inline uint32_t parseTexTarget(uint32_t target) {
    switch (target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_RECTANGLE:
            return GL_TEXTURE_2D;
        case GL_TEXTURE_1D_ARRAY:
            return GL_TEXTURE_2D_ARRAY;
        default:
            return target;
    }
}

static inline uint32_t getTexTargetAt(int index) {
    switch (index) {
        case 0: return GL_TEXTURE_2D;
        case 1: return GL_TEXTURE_3D;
        case 2: return GL_TEXTURE_CUBE_MAP;
        case 3: return GL_TEXTURE_2D_ARRAY;
        default: return GL_NONE;
    }
}

#endif