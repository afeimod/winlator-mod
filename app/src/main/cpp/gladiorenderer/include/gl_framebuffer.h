#ifndef GLADIO_GL_FRAMEBUFFER_H
#define GLADIO_GL_FRAMEBUFFER_H

#include "gladio.h"

#define FLAG_READ_BUFFER_NONE (1<<0)
#define FLAG_READ_BUFFER_COLOR_ATTACHMENT (1<<1)
#define FLAG_DRAW_BUFFER_NONE (1<<9)
#define FLAG_DRAW_BUFFER_COLOR_ATTACHMENT (1<<10)

typedef struct FBAttachmentInfo {
    GLenum type;
    GLuint id;
    uint8_t level;
} FBAttachmentInfo;

typedef struct GLFramebuffer {
    GLuint ownerId;
    GLuint id;
    FBAttachmentInfo colorAttachment[MAX_FB_COLOR_ATTACHMENTS];
    FBAttachmentInfo depthAttachment;
    FBAttachmentInfo stencilAttachment;
    GLuint flags;
} GLFramebuffer;

extern GLuint GLFramebuffer_create();
extern void GLFramebuffer_bind(GLenum target, GLuint id);
extern GLFramebuffer* GLFramebuffer_getBound(GLenum target);
extern void GLFramebuffer_setAttachment(GLenum target, GLenum attachment, GLenum objectType, GLuint objectId, uint8_t level);
extern void GLFramebuffer_delete(GLuint id);
extern void GLFramebuffer_setReadBuffer(GLenum src);
extern void GLFramebuffer_setDrawBuffers(GLuint count, GLenum* dst);

#endif