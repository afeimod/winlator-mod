#ifndef GLADIO_GL_VAO_H
#define GLADIO_GL_VAO_H

#include "gladio.h"
#include "gl_buffer.h"

#define POSITION_ARRAY_INDEX 0
#define COLOR_ARRAY_INDEX 1
#define NORMAL_ARRAY_INDEX 2
#define TEXCOORD_ARRAY_INDEX 3

#define VERTEX_ATTRIB_DISABLED 0
#define VERTEX_ATTRIB_LEGACY_ENABLED 1
#define VERTEX_ATTRIB_ENABLED 2

typedef struct GLVertexAttrib {
#ifdef GL_SERVER
    bool normalized;
    int type;
    GLuint boundArrayBuffer;
    int size;
#endif
    void* pointer;
    short stride;
    uint8_t state;
} GLVertexAttrib;

typedef struct GLVertexArrayObject {
    uint8_t maxEnabledAttribs;
    GLVertexAttrib attribs[VERTEX_ATTRIB_COUNT];
    GLBuffer* buffer[MAX_BUFFER_TARGETS];
    GLuint bgraBuffer;
} GLVertexArrayObject;

#define GL_SEND_VERTEX_ARRAY(requestCode, arrayIdx) \
    GL_CALL_LOCK(); \
    GLClientState* clientState = currentGLContext->clientState; \
    GLuint index = arrayIdx == TEXCOORD_ARRAY_INDEX ? arrayIdx + clientState->activeTexture : arrayIdx; \
    GLBuffer* arrayBuffer = GLBuffer_getBound(GL_ARRAY_BUFFER); \
    if (arrayBuffer) { \
        if (clientState->program > 0) { \
            GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_DISABLED, true); \
            GL_CALL_UNLOCK(); \
            GLboolean normalized = type != GL_FLOAT && type != GL_HALF_FLOAT ? GL_TRUE : GL_FALSE; \
            glVertexAttribPointer(INT32_MAX + index, size, type, normalized, stride, pointer); \
            return; \
        } \
        pointer = arrayBuffer->mappedData + (uint64_t)pointer; \
    } \
    GLVertexArrayObject_setAttribState(clientState, index, VERTEX_ATTRIB_LEGACY_ENABLED, false); \
    clientState->vao->attribs[index].stride = stride > 0 ? stride : (size * sizeofGLType(type)); \
    clientState->vao->attribs[index].pointer = pointer; \
    ArrayBuffer_rewind(&outputBuffer); \
    ArrayBuffer_put(&outputBuffer, size); \
    ArrayBuffer_putInt(&outputBuffer, type); \
    ArrayBuffer_putInt(&outputBuffer, stride); \
    gl_send(currentGLContext->serverRing, requestCode, outputBuffer.buffer, outputBuffer.size); \
    GL_CALL_UNLOCK()

#define GL_READ_VERTEX_ARRAY(arrayIdx) \
    do { \
        GLbyte size = ArrayBuffer_get(&context->inputBuffer); \
        GLenum type = ArrayBuffer_getInt(&context->inputBuffer); \
        GLsizei stride = ArrayBuffer_getInt(&context->inputBuffer); \
        GLClientState* clientState = &currentRenderer->clientState; \
        GLVertexArrayObject_setAttribState(clientState, arrayIdx, VERTEX_ATTRIB_LEGACY_ENABLED, false); \
        clientState->vao->attribs[arrayIdx].size = size; \
        clientState->vao->attribs[arrayIdx].type = type; \
        clientState->vao->attribs[arrayIdx].normalized = type != GL_FLOAT && type != GL_HALF_FLOAT && clientState->program; \
        clientState->vao->attribs[arrayIdx].stride = stride > 0 || clientState->program ? stride : (size * sizeofGLType(type)); \
    } \
    while (0)

extern void GLVertexArrayObject_setAttribState(GLClientState* clientState, GLuint index, uint8_t state, bool clear);
extern void GLVertexArrayObject_setBound(GLClientState* clientState, GLuint id);
extern void GLVertexArrayObject_delete(GLClientState* clientState, GLuint id);

#endif