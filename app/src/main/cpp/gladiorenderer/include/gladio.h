#define HEADER_SIZE 6

#define DEBUG_MODE_GL_ERROR (1<<0)
#define DEBUG_MODE_SHADER_INFO (1<<1)
#define DEBUG_MODE_TEXTURE_FORMAT (1<<2)
#define DEBUG_MODE_HANDLE_REQUEST (1<<3)
#define DEBUG_MODE (0) // FIXME 0=DISABLE or (FIRST | SECOND)
#define IS_DEBUG_ENABLED(type) ((DEBUG_MODE & type) != 0)

#define GLX_EXT_MAJOR_OPCODE -106
#define SERVER_RING_BUFFER_SIZE 67108864
#define CLIENT_RING_BUFFER_SIZE 33554432
#define THREAD_POOL_NUM_THREADS 4
#define PIXEL_READ_CACHE_SKIP_FRAMES 3
#define X11_SERVER_PATH "/data/data/com.winlator/files/rootfs/tmp/.X11-unix/X0"

#define GL_STRING_VERSION "3.3"
#define GL_STRING_RENDERER "Gladio"
#define GL_STRING_SHADING_LANGUAGE_VERSION "3.30"

#include "winlator.h"

#ifdef __ANDROID__
#define GL_SERVER 1
#endif

#define MAX_LIGHTS 4
#define MAX_TEXCOORDS 8
#define MAX_TEXTURE_TARGETS 4
#define MAX_FRAMEBUFFER_TARGETS 3
#define MAX_BUFFER_TARGETS 6
#define MAX_ARB_PROGRAM_TARGETS 2
#define MAX_FB_COLOR_ATTACHMENTS 8
#define VERTEX_ATTRIB_COUNT (3 + MAX_TEXCOORDS)

#define GL_SEND_CHECKED(requestCode, outputBuffer, bufferSize, ...) \
    do { \
        if (!gl_send(currentGLContext->serverRing, requestCode, outputBuffer, bufferSize)) { \
            GL_CALL_UNLOCK(); \
            return __VA_OPT__(GL_FALSE); \
        } \
    } \
    while (0)

#define GL_RECV_CHECKED(...) \
    do { \
        if (!gl_recv(currentGLContext->clientRing, NULL, &inputBuffer)) { \
            GL_CALL_UNLOCK(); \
            return __VA_OPT__(GL_FALSE); \
        } \
    } \
    while (0)

#ifndef GLADIO_H
#define GLADIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "request_codes.h"
#include "socket_utils.h"
#include "arrays.h"
#include "ring_buffer.h"
#include "image_utils.h"
#include "texture_utils.h"
#include "time_utils.h"
#include "string_utils.h"
#include "events.h"

typedef struct GLClientState GLClientState;

#ifdef GL_SERVER

#include <jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

typedef struct GLRenderer GLRenderer;
typedef struct GLContext GLContext;

typedef struct JMethods {
    JavaVM* jvm;
    JNIEnv* env;
    jobject obj;
    jmethodID getWindowSize;
    jmethodID clearWindowContent;
    jmethodID updateWindowContent;
    jmethodID getGLXContextPtr;
} JMethods;

#else // GL_SERVER
#include "GL/gl.h"
#include "GL/glext.h"
#include "GL/glx.h"
#include "gl_dsa.h"
#endif

static inline bool gl_send(RingBuffer* ring, short requestCode, void* data, int size) {
    char header[HEADER_SIZE];
    *(short*)(header + 0) = requestCode;
    *(int*)(header + 2) = size;

    bool result = RingBuffer_write(ring, header, HEADER_SIZE);
    if (!result) return false;

    if (size > 0) {
        result = RingBuffer_write(ring, data, size);
        if (!result) return false;
    }
    return true;
}

static inline bool gl_recv(RingBuffer* ring, short* outRequestCode, ArrayBuffer* data) {
    char header[HEADER_SIZE] = {0};
    bool result = RingBuffer_read(ring, header, HEADER_SIZE);
    if (!result) return false;

    if (outRequestCode != NULL) *outRequestCode = *(short*)(header + 0);
    int size = *(int*)(header + 2);

    ArrayBuffer_rewind(data);

    if (size > 0) {
        ENSURE_ARRAY_CAPACITY(size, data->capacity, data->buffer, 1);
        result = RingBuffer_read(ring, data->buffer, size);
        if (!result) return false;
        data->size = size;
    }
    return true;
}

static inline bool glx_send(int fd, char opcode, void* data, int size) {
    char header[4];
    header[0] = GLX_EXT_MAJOR_OPCODE;
    header[1] = opcode;
    *(short*)(header + 2) = size / 4 + 1;

    bool result = sock_write(fd, header, 4) == 4;
    if (!result) return false;

    if (size > 0) {
        result = sock_write(fd, data, size) == size;
        if (!result) return false;
    }
    return true;
}

static inline bool glx_recv(int fd, ArrayBuffer* data) {
    char header[8] = {0};
    bool result = sock_read(fd, header, sizeof(header)) == sizeof(header);
    if (!result) return false;
    int replyLength = *(int*)(header + 4);

    int size = 24 + replyLength * 4;
    ArrayBuffer_rewind(data);

    ENSURE_ARRAY_CAPACITY(size, data->capacity, data->buffer, 1);
    result = sock_read(fd, data->buffer, size) == size;
    if (!result || header[0] != 1) return false;

    data->size = size;
    return true;
}

static inline int getGLCallParamsCount(GLenum pname, bool* isColor) {
    if (isColor) *isColor = false;
    switch (pname) {
        case GL_PATCH_DEFAULT_INNER_LEVEL:
            return 2;
        case GL_SPOT_DIRECTION:
        case GL_POINT_DISTANCE_ATTENUATION:
            return 3;
        case GL_POSITION:
        case GL_PATCH_DEFAULT_OUTER_LEVEL:
            return 4;
        case GL_AMBIENT:
        case GL_DIFFUSE:
        case GL_SPECULAR:
        case GL_EMISSION:
        case GL_AMBIENT_AND_DIFFUSE:
        case GL_TEXTURE_ENV_COLOR:
        case GL_FOG_COLOR:
        case GL_TEXTURE_BORDER_COLOR:
            if (isColor) *isColor = true;
            return 4;
        default:
            return 1;
    }
}

#define PARSE_PARAMSFV(pname, params) \
    bool isColor; \
    int paramCount = getGLCallParamsCount(pname, &isColor); \
    GLfloat paramsf[paramCount]; \
    for (int p = 0; p < paramCount; p++) paramsf[p] = (isColor ? (GLfloat)params[p] / (double)INT32_MAX : (GLfloat)params[p])

#define PARSE_PARAMSIFV(pname, params) \
    int paramCount = getGLCallParamsCount(pname, NULL); \
    GLfloat paramsf[paramCount]; \
    for (int p = 0; p < paramCount; p++) paramsf[p] = ((GLfloat)params[p])

#define PARSE_MATRIXF(md) \
    GLfloat mf[16]; \
    for (int p = 0; p < 16; p++) (mf[p] = (GLfloat)md[p])

#define GL_SEND_TEXIMAGE(requestCode, format, type, width, height, depth, imageData, compressedSize) \
    do { \
        GLClientState* clientState = currentGLContext->clientState; \
        GLBuffer* pixelUnpackBuffer = GLBuffer_getBound(GL_PIXEL_UNPACK_BUFFER); \
        int dataSize = compressedSize; \
        if (compressedSize == 0) { \
            if (clientState->pixelStore.unpackRowLength > 0) width = clientState->pixelStore.unpackRowLength; \
            if (clientState->pixelStore.unpackImageHeight > 0) height = clientState->pixelStore.unpackImageHeight; \
            dataSize = imageData && !pixelUnpackBuffer ? computeTexImageDataSize(format, type, width, height, depth) : 0; \
            ArrayBuffer_putInt(&outputBuffer, dataSize); \
        } \
        else ArrayBuffer_putInt(&outputBuffer, compressedSize); \
        if (pixelUnpackBuffer) ArrayBuffer_putInt(&outputBuffer, (uint64_t)imageData); \
        GL_SEND_CHECKED(requestCode, outputBuffer.buffer, outputBuffer.size); \
        if (pixelUnpackBuffer) { \
            GL_RECV_CHECKED(); \
        } \
        else if (dataSize > 0) RingBuffer_write(currentGLContext->serverRing, imageData, dataSize); \
    } \
    while (0)

static inline int sizeofGLType(GLenum type) {
    switch (type) {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return sizeof(GLbyte);
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return sizeof(GLshort);
        case GL_INT:
        case GL_UNSIGNED_INT:
            return sizeof(GLint);
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_DOUBLE:
            return sizeof(GLdouble);
        default:
            return 0;
    }
}

static inline uint8_t indexOfGLTarget(GLenum target) {
    switch (target) {
        case GL_TEXTURE_2D:
        case GL_FRAMEBUFFER:
        case GL_VERTEX_PROGRAM_ARB:
        case GL_VERTEX_PROGRAM_TWO_SIDE:
        case GL_ARRAY_BUFFER:
        case GL_TEXTURE_1D:
            return 0;
        case GL_TEXTURE_3D:
        case GL_DRAW_FRAMEBUFFER:
        case GL_FRAGMENT_PROGRAM_ARB:
        case GL_ELEMENT_ARRAY_BUFFER:
            return 1;
        case GL_TEXTURE_CUBE_MAP:
        case GL_READ_FRAMEBUFFER:
        case GL_PIXEL_PACK_BUFFER:
            return 2;
        case GL_PIXEL_UNPACK_BUFFER:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_1D_ARRAY:
            return 3;
        case GL_UNIFORM_BUFFER:
            return 4;
        case GL_TEXTURE_BUFFER:
            return 5;
        default:
            if (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X && target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) return 2;
            println("gladio: unimplemented gl target %x", target);
            return -1;
    }
}

#endif