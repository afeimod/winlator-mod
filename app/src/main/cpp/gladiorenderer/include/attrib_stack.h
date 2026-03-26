#ifndef GLADIO_ATTRIB_STACK_H
#define GLADIO_ATTRIB_STACK_H

#include "gladio.h"

typedef struct AttribStack {
    GLbitfield mask;

    GLboolean alphaTest;
    GLint alphaTestFunc;
    GLclampf alphaTestRef;
    GLboolean blend;
    GLint blendSrc;
    GLint blendDst;
    GLfloat clearColor[4];
    GLboolean colorMask[4];

    GLfloat color[4];
    GLfloat normal[3];
    GLfloat texCoord[4];

    GLboolean depthTest;
    GLint depthFunc;
    GLfloat clearDepth;
    GLint depthMask;

    GLboolean cullFace;

    GLboolean fog;
    GLfloat fogColor[4];
    GLfloat fogDensity;
    GLfloat fogStart;
    GLfloat fogEnd;
    GLfloat fogMode;

    GLboolean scissorTest;
    GLfloat scissorBox[4];

    GLboolean stencilTest;
    GLenum stencilFunc;
    GLint stencilRef;
    GLuint stencilMask;
    GLint clearStencil;
    GLenum stencilFail;
    GLenum stencilPassDepthFail;
    GLenum stencilPassDepthPass;

    uint8_t activeTexture;
    GLuint boundTexture[MAX_TEXTURE_TARGETS];

    GLint viewportSize[4];
    GLfloat depthRange[2];
} AttribStack;

extern void AttribStack_push(GLbitfield mask);
extern void AttribStack_pop();

#endif