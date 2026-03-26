#ifndef GLADIO_GL_CONTEXT_H
#define GLADIO_GL_CONTEXT_H

#include "gladio.h"
#include "gl_renderer.h"
#include "gl_texture.h"

typedef struct GLXContext {
    EGLContext eglContext;
    GLRenderer renderer;
} GLXContext;

typedef struct GLContext {
    GLXContext* glxContext;
    int clientFd;
    RingBuffer* clientRing;
    RingBuffer* serverRing;
    JMethods jmethods;

    pthread_t requestHandlerThread;
    bool running;
    ArrayBuffer inputBuffer;
    ArrayBuffer outputBuffer;

    ThreadPool* threadPool;

    struct {
        GLuint renderbuffer;
        GLuint texture;
    } displayBufAttachments[2];

    GLuint savedDSATarget;
    GLuint savedDSAId;
} GLContext;

extern GLContext* createGLContext(JNIEnv* env, jobject obj, int clientFd);
extern void destroyGLContext(JNIEnv* env, GLContext* context);
extern GLXContext* createGLXContext(int contextId, GLXContext* sharedContext);
extern void destroyGLXContext(GLXContext* glxContext);
extern void readCommandBuffer(GLContext* context);
extern void readVertexArrayElement(GLContext* context, int arrayIdx, int elementIdx);
extern bool readUnboundVertexArrays(GLContext* context, GLenum drawMode, int drawCount, void** outIndices, GLenum indexType);
extern const char* getGLExtensions(int* numExtensions);

extern pthread_mutex_t glx_context_mutex;

#define GLX_CONTEXT_LOCK() pthread_mutex_lock(&glx_context_mutex)
#define GLX_CONTEXT_UNLOCK() pthread_mutex_unlock(&glx_context_mutex)

#endif