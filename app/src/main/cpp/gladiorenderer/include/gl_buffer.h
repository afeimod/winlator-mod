#ifndef GLADIO_GL_BUFFER_H
#define GLADIO_GL_BUFFER_H

#include "gladio.h"

typedef struct GLBuffer {
    GLuint id;
    int size;
    void* mappedData;
} GLBuffer;

extern GLBuffer* GLBuffer_getBound(GLenum target);
extern void GLBuffer_bind(GLenum target, GLuint id);
extern int GLBuffer_allocateMemory(GLBuffer* buffer, int size);
extern void GLBuffer_delete(GLuint id);
extern void GLBuffer_onDestroy(GLClientState* clientState);

#endif