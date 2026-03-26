#include "gl_framebuffer.h"
#include "gl_context.h"

static GLuint maxFramebufferId = 1;

static GLFramebuffer* createNamedFramebuffer(GLuint id) {
    GLFramebuffer* framebuffer = calloc(1, sizeof(GLFramebuffer));
    framebuffer->ownerId = currentRenderer->contextId;
    framebuffer->id = id;
    SparseArray_put(currentRenderer->clientState.framebuffers, id, framebuffer);
    return framebuffer;
}

GLuint GLFramebuffer_create() {
    GLX_CONTEXT_LOCK();
    GLuint id = maxFramebufferId++;
    createNamedFramebuffer(id);
    GLX_CONTEXT_UNLOCK();
    return id;
}

static void assignFBAttachment(GLenum target, GLuint attachment, FBAttachmentInfo* attachmentInfo) {
    if (attachmentInfo->type == 0) return;
    if (attachmentInfo->type == GL_RENDERBUFFER) {
        glFramebufferRenderbuffer(target, attachment, attachmentInfo->type, attachmentInfo->id);
    }
    else glFramebufferTexture2D(target, attachment, attachmentInfo->type, attachmentInfo->id, attachmentInfo->level);
}

static void recreateFramebuffer(GLenum target, GLFramebuffer* framebuffer) {
    glBindFramebuffer(target, framebuffer->id);

    for (int i = 0; i < MAX_FB_COLOR_ATTACHMENTS; i++) {
        assignFBAttachment(target, GL_COLOR_ATTACHMENT0 + i, &framebuffer->colorAttachment[i]);
    }

    assignFBAttachment(target, GL_DEPTH_ATTACHMENT, &framebuffer->depthAttachment);
    assignFBAttachment(target, GL_STENCIL_ATTACHMENT, &framebuffer->stencilAttachment);

    if (framebuffer->flags & FLAG_READ_BUFFER_NONE) glReadBuffer(GL_NONE);
    for (int i = 0, j = GETEXP(FLAG_READ_BUFFER_COLOR_ATTACHMENT); i < MAX_FB_COLOR_ATTACHMENTS; i++, j++) {
        if (framebuffer->flags & (1<<j)) glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
    }

    if (framebuffer->flags & FLAG_DRAW_BUFFER_NONE) {
        GLenum none = GL_NONE;
        glDrawBuffers(1, &none);
    }

    GLenum drawBufs[MAX_FB_COLOR_ATTACHMENTS];
    int numDrawBufs = 0;
    for (int i = 0, j = GETEXP(FLAG_DRAW_BUFFER_COLOR_ATTACHMENT); i < MAX_FB_COLOR_ATTACHMENTS; i++, j++) {
        if (framebuffer->flags & (1<<j)) drawBufs[numDrawBufs++] = GL_COLOR_ATTACHMENT0 + i;
    }
    if (numDrawBufs > 0) glDrawBuffers(numDrawBufs, drawBufs);
}

void GLFramebuffer_bind(GLenum target, GLuint id) {
    GLX_CONTEXT_LOCK();
    if (id == 0) id = currentRenderer->displayBuffers[1];
    GLFramebuffer* framebuffer = SparseArray_get(currentRenderer->clientState.framebuffers, id);
    if (!framebuffer) framebuffer = createNamedFramebuffer(id);

    if (target == GL_FRAMEBUFFER) {
        ARRAYS_FILL(currentRenderer->clientState.framebuffer, MAX_FRAMEBUFFER_TARGETS, id);
    }
    else currentRenderer->clientState.framebuffer[indexOfGLTarget(target)] = id;

    if (currentRenderer->contextId != framebuffer->ownerId) {
        recreateFramebuffer(target, framebuffer);
        framebuffer->ownerId = currentRenderer->contextId;
    }
    else glBindFramebuffer(target, framebuffer->id);
    GLX_CONTEXT_UNLOCK();
}

GLFramebuffer* GLFramebuffer_getBound(GLenum target) {
    GLX_CONTEXT_LOCK();
    GLuint id = currentRenderer->clientState.framebuffer[indexOfGLTarget(target)];
    GLFramebuffer* framebuffer = SparseArray_get(currentRenderer->clientState.framebuffers, id);
    GLX_CONTEXT_UNLOCK();
    return framebuffer;
}

void GLFramebuffer_setAttachment(GLenum target, GLenum attachment, GLenum objectType, GLuint objectId, uint8_t level) {
    GLFramebuffer* framebuffer = GLFramebuffer_getBound(target);

    if (attachment >= GL_COLOR_ATTACHMENT0 && attachment <= GL_COLOR_ATTACHMENT31) {
        int index = attachment - GL_COLOR_ATTACHMENT0;
        framebuffer->colorAttachment[index].type = objectType;
        framebuffer->colorAttachment[index].id = objectId;
        framebuffer->colorAttachment[index].level = level;
        assignFBAttachment(target, attachment, &framebuffer->colorAttachment[index]);
    }
    else {
        if (attachment == GL_DEPTH_ATTACHMENT ||
            attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
            framebuffer->depthAttachment.type = objectType;
            framebuffer->depthAttachment.id = objectId;
            framebuffer->depthAttachment.level = level;
            assignFBAttachment(target, attachment, &framebuffer->depthAttachment);
        }
        if (attachment == GL_STENCIL_ATTACHMENT ||
            attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
            framebuffer->stencilAttachment.type = objectType;
            framebuffer->stencilAttachment.id = objectId;
            framebuffer->stencilAttachment.level = level;
            assignFBAttachment(target, attachment, &framebuffer->stencilAttachment);
        }
    }
}

void GLFramebuffer_delete(GLuint id) {
    for (int i = 0; i < MAX_FRAMEBUFFER_TARGETS; i++) {
        if (id == currentRenderer->clientState.framebuffer[i]) {
            GLFramebuffer_bind(GL_FRAMEBUFFER, 0);
        }
    }

    GLX_CONTEXT_LOCK();
    GLFramebuffer* framebuffer = SparseArray_get(currentRenderer->clientState.framebuffers, id);
    if (framebuffer && framebuffer->ownerId == currentRenderer->contextId) {
        glDeleteFramebuffers(1, &framebuffer->id);
        SparseArray_remove(currentRenderer->clientState.framebuffers, id);
        free(framebuffer);
    }
    GLX_CONTEXT_UNLOCK();
}

void GLFramebuffer_setReadBuffer(GLenum src) {
    GLFramebuffer* framebuffer = GLFramebuffer_getBound(GL_FRAMEBUFFER);
    BITMASK_UNSET(framebuffer->flags, FLAG_READ_BUFFER_NONE);

    for (int i = 0, j = GETEXP(FLAG_READ_BUFFER_COLOR_ATTACHMENT); i < MAX_FB_COLOR_ATTACHMENTS; i++, j++) {
        BITMASK_UNSET(framebuffer->flags, (1<<j));
    }

    bool success = false;
    if (src == GL_NONE) {
        BITMASK_SET(framebuffer->flags, FLAG_READ_BUFFER_NONE);
        success = true;
    }
    else if (src >= GL_COLOR_ATTACHMENT0 && src <= GL_COLOR_ATTACHMENT31) {
        int index = (src - GL_COLOR_ATTACHMENT0) + GETEXP(FLAG_READ_BUFFER_COLOR_ATTACHMENT);
        BITMASK_SET(framebuffer->flags, (1<<index));
        success = true;
    }

    if (success) glReadBuffer(src);
}

void GLFramebuffer_setDrawBuffers(GLuint count, GLenum* dst) {
    GLFramebuffer* framebuffer = GLFramebuffer_getBound(GL_FRAMEBUFFER);
    BITMASK_UNSET(framebuffer->flags, FLAG_DRAW_BUFFER_NONE);

    for (int i = 0, j = GETEXP(FLAG_DRAW_BUFFER_COLOR_ATTACHMENT); i < MAX_FB_COLOR_ATTACHMENTS; i++, j++) {
        BITMASK_UNSET(framebuffer->flags, (1<<j));
    }

    bool success = false;
    for (int i = 0; i < count; i++) {
        if (dst[i] == GL_NONE) {
            BITMASK_SET(framebuffer->flags, FLAG_DRAW_BUFFER_NONE);
            success = true;
        }
        else if (dst[i] >= GL_COLOR_ATTACHMENT0 && dst[i] <= GL_COLOR_ATTACHMENT31) {
            int index = (dst[i] - GL_COLOR_ATTACHMENT0) + GETEXP(FLAG_DRAW_BUFFER_COLOR_ATTACHMENT);
            BITMASK_SET(framebuffer->flags, (1<<index));
            success = true;
        }
    }
    if (success) glDrawBuffers(count, dst);
}