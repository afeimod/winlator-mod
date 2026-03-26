#include "gl_formats.h"

static GLFormatInfo textureFormats[] = {
    {GL_R8, GL_RED, GL_UNSIGNED_NORMALIZED, 8, 0, 0, 0, 0, true, true},
    {GL_R8_SNORM, GL_RED, GL_SIGNED_NORMALIZED, 8, 0, 0, 0, 0, false, true},
    {GL_R16F, GL_RED, GL_FLOAT, 16, 0, 0, 0, 0, false, true},
    {GL_R32F, GL_RED, GL_FLOAT, 32, 0, 0, 0, 0, false, false},
    {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 8, 0, 0, 0, 0, true, false},
    {GL_R8I, GL_RED_INTEGER, GL_INT, 8, 0, 0, 0, 0, true, false},
    {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 16, 0, 0, 0, 0, true, false},
    {GL_R16I, GL_RED_INTEGER, GL_INT, 16, 0, 0, 0, 0, true, false},
    {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 32, 0, 0, 0, 0, true, false},
    {GL_R32I, GL_RED_INTEGER, GL_INT, 32, 0, 0, 0, 0, true, false},
    {GL_RG8, GL_RG, GL_UNSIGNED_NORMALIZED, 8, 8, 0, 0, 0, true, true},
    {GL_RG8_SNORM, GL_RG, GL_SIGNED_NORMALIZED, 8, 8, 0, 0, 0, false, true},
    {GL_RG16F, GL_RG, GL_FLOAT, 16, 16, 0, 0, 0, false, true},
    {GL_RG32F, GL_RG, GL_FLOAT, 32, 32, 0, 0, 0, false, false},
    {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 8, 8, 0, 0, 0, true, false},
    {GL_RG8I, GL_RG_INTEGER, GL_INT, 8, 8, 0, 0, 0, true, false},
    {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 16, 16, 0, 0, 0, true, false},
    {GL_RG16I, GL_RG_INTEGER, GL_INT, 16, 16, 0, 0, 0, true, false},
    {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 32, 32, 0, 0, 0, true, false},
    {GL_RG32I, GL_RG_INTEGER, GL_INT, 32, 32, 0, 0, 0, true, false},
    {GL_RGB8, GL_RGB, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 0, 0, true, true},
    {GL_SRGB8, GL_RGB, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 0, 0, false, true},
    {GL_RGB565, GL_RGB, GL_UNSIGNED_NORMALIZED, 5, 6, 5, 0, 0, true, true},
    {GL_RGB8_SNORM, GL_RGB, GL_SIGNED_NORMALIZED, 8, 8, 8, 0, 0, false, true},
    {GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT, 11, 11, 10, 0, 0, false, true},
    {GL_RGB9_E5, GL_RGB, GL_UNSIGNED_NORMALIZED, 9, 9, 9, 0, 5, false, true},
    {GL_RGB16F, GL_RGB, GL_FLOAT, 16, 16, 16, 0, 0, false, true},
    {GL_RGB32F, GL_RGB, GL_FLOAT, 32, 32, 32, 0, 0, false, false},
    {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 8, 8, 8, 0, 0, false, false},
    {GL_RGB8I, GL_RGB_INTEGER, GL_INT, 8, 8, 8, 0, 0, false, false},
    {GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 16, 16, 16, 0, 0, false, false},
    {GL_RGB16I, GL_RGB_INTEGER, GL_INT, 16, 16, 16, 0, 0, false, false},
    {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 32, 32, 32, 0, 0, false, false},
    {GL_RGB32I, GL_RGB_INTEGER, GL_INT, 32, 32, 32, 0, 0, false, false},
    {GL_RGBA8, GL_RGBA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 8, 0, true, true},
    {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 8, 0, true, true},
    {GL_RGBA8_SNORM, GL_RGBA, GL_SIGNED_NORMALIZED, 8, 8, 8, 8, 0, false, true},
    {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_NORMALIZED, 5, 5, 5, 1, 0, true, true},
    {GL_RGBA4, GL_RGBA, GL_UNSIGNED_NORMALIZED, 4, 4, 4, 4, 0, true, true},
    {GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_NORMALIZED, 10, 10, 10, 2, 0, true, true},
    {GL_RGBA16F, GL_RGBA, GL_FLOAT, 16, 16, 16, 16, 0, false, true},
    {GL_RGBA32F, GL_RGBA, GL_FLOAT, 32, 32, 32, 32, 0, false, false},
    {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 8, 8, 8, 8, 0, true, false},
    {GL_RGBA8I, GL_RGBA_INTEGER, GL_INT, 8, 8, 8, 8, 0, true, false},
    {GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 10, 10, 10, 2, 0, true, false},
    {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 16, 16, 16, 16, 0, true, false},
    {GL_RGBA16I, GL_RGBA_INTEGER, GL_INT, 16, 16, 16, 16, 0, true, false},
    {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 32, 32, 32, 32, 0, true, false},
    {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 32, 32, 32, 32, 0, true, false},
    {GL_BGRA, GL_BGRA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 8, 0, true, true},
    {GL_LUMINANCE8_ALPHA8, GL_RG, GL_UNSIGNED_NORMALIZED, 8, 8, 0, 0, 0, true, true},
    {GL_ALPHA8, GL_RED, GL_UNSIGNED_NORMALIZED, 8, 0, 0, 0, 0, true, true},
    {GL_LUMINANCE_LEGACY, GL_RED, GL_UNSIGNED_NORMALIZED, 8, 0, 0, 0, 0, true, true},
    {GL_LUMINANCE_ALPHA_LEGACY, GL_RG, GL_UNSIGNED_NORMALIZED, 8, 8, 0, 0, 0, true, true},
    {GL_RGB_LEGACY, GL_RGB, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 0, 0, true, true},
    {GL_RGBA_LEGACY, GL_RGBA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 0, 0, true, true},
    {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_NORMALIZED, 16, 0, 0, 0, 0, false, false},
    {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_NORMALIZED, 24, 0, 0, 0, 0, false, false},
    {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 32, 0, 0, 0, 0, false, false},
    {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT, 24, 8, 0, 0, 0, false, false},
    {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT, 32, 8, 0, 0, 0, false, false},
};

static GLFormatInfo renderbufferFormats[] = {
    {GL_R8, GL_RED, GL_UNSIGNED_NORMALIZED, 8, 0, 0, 0, 0, true, true},
    {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 8, 0, 0, 0, 0, true, true},
    {GL_R8I, GL_RED_INTEGER, GL_INT, 8, 0, 0, 0, 0, true, true},
    {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 16, 0, 0, 0, 0, true, true},
    {GL_R16I, GL_RED_INTEGER, GL_INT, 16, 0, 0, 0, 0, true, true},
    {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 32, 0, 0, 0, 0, true, true},
    {GL_R32I, GL_RED_INTEGER, GL_INT, 32, 0, 0, 0, 0, true, true},
    {GL_RG8, GL_RG, GL_UNSIGNED_NORMALIZED, 8, 8, 0, 0, 0, true, true},
    {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 8, 8, 0, 0, 0, true, true},
    {GL_RG8I, GL_RG_INTEGER, GL_INT, 8, 8, 0, 0, 0, true, true},
    {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 16, 16, 0, 0, 0, true, true},
    {GL_RG16I, GL_RG_INTEGER, GL_INT, 16, 16, 0, 0, 0, true, true},
    {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 32, 32, 0, 0, 0, true, true},
    {GL_RG32I, GL_RG_INTEGER, GL_INT, 32, 32, 0, 0, 0, true, true},
    {GL_RGB8, GL_RGB, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 0, 0, true, true},
    {GL_RGB565, GL_RGB, GL_UNSIGNED_NORMALIZED, 5, 6, 5, 0, 0, true, true},
    {GL_RGBA8, GL_RGBA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 8, 0, true, true},
    {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_NORMALIZED, 8, 8, 8, 8, 0, true, true},
    {GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_NORMALIZED, 5, 5, 5, 1, 0, true, true},
    {GL_RGBA4, GL_RGBA, GL_UNSIGNED_NORMALIZED, 4, 4, 4, 4, 0, true, true},
    {GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_NORMALIZED, 10, 10, 10, 2, 0, true, true},
    {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 8, 8, 8, 8, 0, true, true},
    {GL_RGBA8I, GL_RGBA_INTEGER, GL_INT, 8, 8, 8, 8, 0, true, true},
    {GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 10, 10, 10, 2, 0, true, true},
    {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 16, 16, 16, 16, 0, true, true},
    {GL_RGBA16I, GL_RGBA_INTEGER, GL_INT, 16, 16, 16, 16, 0, true, true},
    {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 32, 32, 32, 32, 0, true, true},
    {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 32, 32, 32, 32, 0, true, true},
    {GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_NORMALIZED, 16, 0, 0, 0, 0, false, false},
    {GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_NORMALIZED, 24, 0, 0, 0, 0, false, false},
    {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 32, 0, 0, 0, 0, false, false},
    {GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT, 24, 8, 0, 0, 0, false, false},
    {GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT, 32, 8, 0, 0, 0, false, false},
    {GL_STENCIL_INDEX8, GL_STENCIL, GL_UNSIGNED_NORMALIZED, 0, 8, 0, 0, 0, false, false},
};

void GLFormats_queryInternalformat(GLenum target, GLenum internalformat, GLenum pname, int count, int* params) {
    target = parseTexTarget(target);
    int result = GL_NONE;

    GLFormatInfo* targetFormat = NULL;
    if (target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP) {
        for (int i = 0; i < ARRAY_SIZE(textureFormats); i++) {
            if (textureFormats[i].internalformat == internalformat) {
                targetFormat = &textureFormats[i];
                break;
            }
        }
    }
    else if (target == GL_RENDERBUFFER) {
        for (int i = 0; i < ARRAY_SIZE(renderbufferFormats); i++) {
            if (renderbufferFormats[i].internalformat == internalformat) {
                targetFormat = &renderbufferFormats[i];
                break;
            }
        }
    }

    if (targetFormat) {
        switch (pname) {
            case GL_VERTEX_TEXTURE:
            case GL_FRAGMENT_TEXTURE:
            case GL_SHADER_IMAGE_STORE:
            case GL_SRGB_READ:
            case GL_SRGB_WRITE:
                result = GL_FULL_SUPPORT;
                break;
            case GL_FILTER:
                if (targetFormat->textureFilterable) result = GL_FULL_SUPPORT;
                break;
            case GL_FRAMEBUFFER_RENDERABLE:
            case GL_FRAMEBUFFER_BLEND:
                if (targetFormat->colorRenderable) result = GL_FULL_SUPPORT;
                break;
            case GL_INTERNALFORMAT_RED_SIZE:
                result = targetFormat->redSize;
                break;
            case GL_INTERNALFORMAT_GREEN_SIZE:
                result = targetFormat->greenSize;
                break;
            case GL_INTERNALFORMAT_BLUE_SIZE:
                result = targetFormat->blueSize;
                break;
            case GL_INTERNALFORMAT_ALPHA_SIZE:
                result = targetFormat->alphaSize;
                break;
            case GL_INTERNALFORMAT_SHARED_SIZE:
                result = targetFormat->sharedSize;
                break;
            case GL_INTERNALFORMAT_DEPTH_SIZE:
                result = targetFormat->depthSize;
                break;
            case GL_INTERNALFORMAT_STENCIL_SIZE:
                result = targetFormat->stencilSize;
                break;
            case GL_INTERNALFORMAT_RED_TYPE:
                result = targetFormat->redSize > 0 ? targetFormat->componentType : 0;
                break;
            case GL_INTERNALFORMAT_GREEN_TYPE:
                result = targetFormat->greenSize > 0 ? targetFormat->componentType : 0;
                break;
            case GL_INTERNALFORMAT_BLUE_TYPE:
                result = targetFormat->blueSize > 0 ? targetFormat->componentType : 0;
                break;
            case GL_INTERNALFORMAT_ALPHA_TYPE:
                result = targetFormat->alphaSize > 0 ? targetFormat->componentType : 0;
                break;
            case GL_INTERNALFORMAT_DEPTH_TYPE:
                result = targetFormat->depthSize > 0 ? targetFormat->componentType : 0;
                break;
            case GL_INTERNALFORMAT_STENCIL_TYPE:
                result = targetFormat->stencilSize > 0 ? GL_UNSIGNED_NORMALIZED : 0;
                break;
            case GL_NUM_SAMPLE_COUNTS:
            case GL_SAMPLES:
                result = 0;
                break;
            default:
                println("gladio:queryInternalformat: unimplemented pname %x", pname);
                break;
        }
    }

    *params = result;
}