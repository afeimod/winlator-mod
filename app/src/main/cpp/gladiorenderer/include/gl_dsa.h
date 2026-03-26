#ifndef GLADIO_GL_DSA_H
#define GLADIO_GL_DSA_H

#include "gl_context.h"

static inline void handleDSARequest(GLContext* context, short requestCode) {
    switch (requestCode) {
        case REQUEST_CODE_GL_DSA_SAVE_ACTIVE_TEXTURE: {
            GLuint unit = ArrayBuffer_getInt(&context->inputBuffer);
            GLuint oldUnit = GL_TEXTURE0 + currentRenderer->clientState.activeTexture;
            if (unit != oldUnit) {
                GLTexture_setActiveUnit(unit);
                context->savedDSAId = oldUnit;
            }
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_ACTIVE_TEXTURE:
            if (context->savedDSAId > 0)  {
                GLTexture_setActiveUnit(context->savedDSAId);
                context->savedDSAId = 0;
            }
            break;
        case REQUEST_CODE_GL_DSA_SAVE_BOUND_TEXTURE: {
            GLuint texture = ArrayBuffer_getInt(&context->inputBuffer);
            GLenum target = GLTexture_getType(texture);
            GLTexture* oldTexture = GLTexture_getBound(target);
            context->savedDSATarget = target;
            context->savedDSAId = oldTexture ? oldTexture->id : 0;
            GLTexture_bind(target, texture);
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_BOUND_TEXTURE:
            GLTexture_bind(context->savedDSATarget, context->savedDSAId);
            context->savedDSATarget = 0;
            context->savedDSAId = 0;
            break;
        case REQUEST_CODE_GL_DSA_SAVE_BOUND_BUFFER: {
            uint64_t requestData = ArrayBuffer_getLong(&context->inputBuffer);
            UNPACK32(requestData, target, buffer);
            GLBuffer* oldBuffer = GLBuffer_getBound(target);
            context->savedDSATarget = target;
            context->savedDSAId = oldBuffer ? oldBuffer->id : 0;
            GLBuffer_bind(target, buffer);
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_BOUND_BUFFER:
            GLBuffer_bind(context->savedDSATarget, context->savedDSAId);
            context->savedDSATarget = 0;
            context->savedDSAId = 0;
            break;
        case REQUEST_CODE_GL_DSA_SAVE_BOUND_ARB_PROGRAM: {
            GLuint program = ArrayBuffer_getInt(&context->inputBuffer);
            GLenum target = ARBProgram_get(program)->type;
            ARBProgram* oldProgram = ARBProgram_getBound(target);
            context->savedDSATarget = target;
            context->savedDSAId = oldProgram ? oldProgram->id : 0;
            ARBProgram_bind(target, program);
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_BOUND_ARB_PROGRAM:
            ARBProgram_bind(context->savedDSATarget, context->savedDSAId);
            context->savedDSATarget = 0;
            context->savedDSAId = 0;
            break;
        case REQUEST_CODE_GL_DSA_SAVE_BOUND_FRAMEBUFFER: {
            GLuint framebuffer = ArrayBuffer_getInt(&context->inputBuffer);
            context->savedDSAId = currentRenderer->clientState.framebuffer[indexOfGLTarget(GL_FRAMEBUFFER)];
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_BOUND_FRAMEBUFFER:
            glBindFramebuffer(GL_FRAMEBUFFER, context->savedDSAId);
            context->savedDSAId = 0;
            break;
        case REQUEST_CODE_GL_DSA_SAVE_BOUND_RENDERBUFFER: {
            GLuint renderbuffer = ArrayBuffer_getInt(&context->inputBuffer);
            context->savedDSAId = currentRenderer->clientState.renderbuffer;
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
            break;
        }
        case REQUEST_CODE_GL_DSA_RESTORE_BOUND_RENDERBUFFER:
            glBindRenderbuffer(GL_RENDERBUFFER, context->savedDSAId);
            context->savedDSAId = 0;
            break;
    }
}

#endif
