#include "gl_context.h"
#include "shader_converter.h"

JNIEXPORT jlong JNICALL
Java_com_winlator_xserver_extensions_GLXExtension_createGLContext(JNIEnv *env, jobject obj,
                                                                  jint clientFd) {
    GLContext* context = createGLContext(env, obj, clientFd);
    return (jlong)context;
}

JNIEXPORT void JNICALL
Java_com_winlator_xserver_extensions_GLXExtension_destroyGLContext(JNIEnv *env, jobject obj,
                                                                   jlong contextPtr) {
    destroyGLContext(env, (GLContext*)contextPtr);
}

JNIEXPORT jlong JNICALL
Java_com_winlator_xserver_extensions_GLXExtension_createGLXContext(JNIEnv *env, jobject obj,
                                                                   jint contextId, jlong sharedContextPtr) {
    GLXContext* context = createGLXContext(contextId, (GLXContext*)sharedContextPtr);
    return (jlong)context;
}

JNIEXPORT void JNICALL
Java_com_winlator_xserver_extensions_GLXExtension_destroyGLXContext(JNIEnv *env, jobject obj,
                                                                    jlong contextPtr) {
    destroyGLXContext((GLXContext*)contextPtr);
}