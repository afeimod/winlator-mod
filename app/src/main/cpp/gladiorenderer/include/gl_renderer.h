#ifndef GLADIO_GL_RENDERER_H
#define GLADIO_GL_RENDERER_H

#include <EGL/egl.h>

#include "shader_material.h"
#include "shader_converter.h"
#include "gl_texture.h"
#include "mat4.h"
#include "vec3.h"
#include "compressed_texture.h"
#include "gl_buffer.h"
#include "gl_client_state.h"
#include "gl_query.h"

#define MODEL_VIEW_MATRIX_INDEX 0
#define PROJECTION_MATRIX_INDEX 1
#define TEXTURE_MATRIX_INDEX 2

typedef struct GLLight {
    bool enabled;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float position[4];
    float attenuation[3];
    float spotCutoffExponent[2];
    float spotDirection[3];
} GLLight;

typedef struct GLMaterial {
    float ambient[3];
    float diffuse[3];
    float specular[4];
    float emission[3];
} GLMaterial;

typedef struct Mesh {
    GLenum mode;
    int start;
    int end;
    int indexCount;
} Mesh;

typedef struct Geometry {
    ArrayBuffer vertices;
    ArrayBuffer colors;
    ArrayBuffer normals;
    ArrayBuffer texCoords[MAX_TEXCOORDS];
    ArrayBuffer indices;
} Geometry;

typedef struct GLRaster {
    float position[4];
    float pixelZoom[2];
    GLuint textureId;

    struct {
        ArrayBuffer vertices;
        ArrayBuffer texCoords;
    } quad;
} GLRaster;

typedef struct TexEnv {
    GLenum mode;
    GLfloat color[4];
    GLint combineRGBA[2];
    GLfloat rgbaScale[2];
    GLint sourceRGBA[4];
    GLint operandRGBA[4];
} TexEnv;

typedef struct GLState {
    bool lighting;
    GLenum polygonMode;

    struct {
        bool enabled;
        GLenum face;
        GLenum mode;
    } colorMaterial;

    struct {
        bool enabled;
        GLenum func;
        float ref;
    } alphaTest;

    struct {
        float color[4];
        float density;
        float start;
        float end;
        GLenum mode;
        bool enabled;
    } fog;

    struct {
        float size;
        float sizeMin;
        float sizeMax;
        float fadeThresholdSize;
        float distanceAttenuation[3];
        GLenum spriteCoordOrigin;
        bool sprite;
        bool smooth;
    } point;

    float color[4];
    float normal[3];
    float texCoords[MAX_TEXCOORDS][4];
    TexEnv texEnv[MAX_TEXCOORDS];
    bool enabledTextures[MAX_TEXCOORDS][MAX_TEXTURE_TARGETS];
    bool enabledARBPrograms[2];

    GLenum shadeModel;
    bool clampReadColor;
} GLState;

typedef struct GLRenderer {
    int contextId;
    short displaySize[2];
    GLuint displayBuffers[2];
    bool swapBuffers;
    GLuint bufferIds[VERTEX_ATTRIB_COUNT+1];

    GLState state;
    GLClientState clientState;

    Geometry geometry;
    GLMaterial* materials;

    GLLight lights[MAX_LIGHTS];
    SparseArray materialMap;
    ArrayDeque meshes;
    Mesh* activeMesh;

    ArrayList matrixStack[3];
    GLuint matrixIndex;

    GLRaster* raster;
    ArrayList attribStacks;
    bool enabledVertexAttribs[VERTEX_ATTRIB_COUNT];
    GLuint circleTexture;

    GLQuery* activeQuery;
    uint64_t queriesStartTime;
} GLRenderer;

extern void GLRenderer_initOnEGLContext(GLRenderer* renderer);
extern void GLRenderer_drawImmediate(GLRenderer* renderer);
extern void GLRenderer_beginImmediate(GLRenderer* renderer, GLenum mode);
extern void GLRenderer_endImmediate(GLRenderer* renderer);
extern float* GLRenderer_getMatrixFromStack(GLRenderer* renderer, int index);
extern float* GLRenderer_getCurrentMatrix(GLRenderer* renderer);
extern void GLRenderer_pushMatrix(GLRenderer* renderer);
extern void GLRenderer_popMatrix(GLRenderer* renderer);
extern void GLRenderer_addVertex(GLRenderer* renderer, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void GLRenderer_addArrayElement(GLRenderer* renderer, int index);
extern void GLRenderer_setCapabilityState(GLRenderer* renderer, GLenum cap, bool state, int index);
extern void GLRenderer_setMaterialParams(GLRenderer* renderer, GLenum face, GLenum pname, void* params);
extern void GLRenderer_setLightParams(GLRenderer* renderer, GLenum id, GLenum pname, void* params);
extern void GLRenderer_setTexEnvParams(GLRenderer* renderer, GLenum target, GLenum pname, float* params);
extern void GLRenderer_setFogParams(GLRenderer* renderer, GLenum pname, GLfloat* params);
extern void GLRenderer_setPointParams(GLRenderer* renderer, GLenum pname, void* params);
extern void GLRenderer_destroy(GLRenderer* renderer);
extern int GLRenderer_getParamsv(GLRenderer* renderer, GLenum pname, GLenum type, void* params);
extern void GLRenderer_setPixelZoom(GLRenderer* renderer, GLfloat xfactor, GLfloat yfactor);
extern void GLRenderer_setRasterPos(GLRenderer* renderer, bool transform, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void GLRenderer_drawPixels(GLRenderer* renderer, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
extern void GLRenderer_setSamplerParameter(GLRenderer* renderer, GLuint sampler, GLenum pname, GLfloat* params);
extern void GLRenderer_getSamplerParameter(GLRenderer* renderer, GLuint sampler, GLenum pname, GLfloat* params);
extern void GLRenderer_setTexParameter(GLRenderer* renderer, GLenum target, GLenum pname, GLfloat* params);
extern void GLRenderer_getTexParameter(GLRenderer* renderer, GLenum target, GLint level, GLenum pname, GLfloat* params);
extern void* GLRenderer_getTexImage(GLRenderer* renderer, GLenum target, GLint level, GLenum format, GLenum type, int* imageSize);
extern void* GLRenderer_getCompressedTexImage(GLRenderer* renderer, GLenum target, GLint level, int* compressedSize);
extern void GLRenderer_setDrawBuffer(GLRenderer* renderer, GLenum drawBuffer);
extern void GLRenderer_enableVertexAttribute(GLRenderer* renderer, int location);
extern void GLRenderer_disableVertexAttribute(GLRenderer* renderer, int location);
extern void GLRenderer_disableUnusedVertexAttributes(GLRenderer* renderer);
extern void GLRenderer_readPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);

extern thread_local GLRenderer* currentRenderer;

#endif
