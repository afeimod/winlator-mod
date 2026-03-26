#ifndef GLADIO_ARB_PROGRAM_H
#define GLADIO_ARB_PROGRAM_H

#include "gladio.h"

typedef struct ARBProgram {
    GLuint id;
    GLenum type;
    GLuint shaderId;
    GLuint threadId;
    SparseArray envParams;
    SparseArray localParams;
    char* shaderCode;
    char* asmSource;
} ARBProgram;

extern ARBProgram* ARBProgram_create();
extern ARBProgram* ARBProgram_get(GLuint programId);
extern ARBProgram* ARBProgram_getBound(GLenum target);
extern void ARBProgram_bind(GLenum target, GLuint programId);
extern void ARBProgram_setSource(ARBProgram* program, GLenum format, char* string, GLuint length);
extern void ARBProgram_delete(GLuint programId);
extern void ARBProgram_setEnvParameter(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void ARBProgram_setLocalParameter(ARBProgram* program, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void ARBProgram_onDestroy(GLClientState* clientState);

#endif