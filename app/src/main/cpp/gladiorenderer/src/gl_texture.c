#include "gl_texture.h"
#include "gl_context.h"

GLTexture* GLTexture_getBound(GLenum target) {
    return currentRenderer->clientState.texture[indexOfGLTarget(target)];
}

void GLTexture_bind(GLenum target, GLuint id) {
    if (id == 0) {
        currentRenderer->clientState.texture[indexOfGLTarget(target)] = NULL;
        glBindTexture(target, 0);
        return;
    }
    GLX_CONTEXT_LOCK();
    GLTexture* texture = SparseArray_get(currentRenderer->clientState.textures, id);
    if (!texture) {
        texture = calloc(1, sizeof(GLTexture));
        texture->id = id;
        texture->type = target;
        SparseArray_put(currentRenderer->clientState.textures, id, texture);
    }
    GLX_CONTEXT_UNLOCK();
    currentRenderer->clientState.texture[indexOfGLTarget(target)] = texture;
    glBindTexture(target, id);
}

void GLTexture_setActiveUnit(GLenum unit) {
    currentRenderer->clientState.activeTexture = unit - GL_TEXTURE0;
    glActiveTexture(unit);
}

GLenum GLTexture_getType(GLuint id) {
    GLX_CONTEXT_LOCK();
    GLTexture* texture = SparseArray_get(currentRenderer->clientState.textures, id);
    GLX_CONTEXT_UNLOCK();
    return texture ? texture->type : GL_NONE;
}

void GLTexture_delete(GLuint id) {
    GLX_CONTEXT_LOCK();
    GLTexture* texture = SparseArray_get(currentRenderer->clientState.textures, id);
    if (texture) {
        for (int i = 0; i < MAX_TEXTURE_TARGETS; i++) {
            if (texture == currentRenderer->clientState.texture[i]) currentRenderer->clientState.texture[i] = NULL;
        }

        SparseArray_remove(currentRenderer->clientState.textures, id);
        free(texture);
    }
    glDeleteTextures(1, &id);
    GLX_CONTEXT_UNLOCK();
}