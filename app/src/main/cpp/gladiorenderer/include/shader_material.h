#ifndef GLADIO_SHADER_MATERIAL_H
#define GLADIO_SHADER_MATERIAL_H

#include "gladio.h"
#include "arb_program.h"

#define TEXENV_MODE_MODULATE "8448"
#define TEXENV_MODE_DECAL "8449"
#define TEXENV_MODE_INTERPOLATE "34165"
#define TEXENV_MODE_ADD "260"
#define TEXENV_MODE_ADD_SIGNED "34164"
#define TEXENV_MODE_SUBTRACT "34023"
#define TEXENV_MODE_COMBINE "34160"
#define TEXENV_MODE_DOT3_RGB "34478"
#define TEXENV_MODE_DOT3_RGBA "34479"
#define TEXENV_MODE_REPLACE "7681"
#define TEXENV_COMBINE_TEXTURE "5890"
#define TEXENV_COMBINE_CONSTANT "34166"
#define TEXENV_COMBINE_PRIMARY_COLOR "34167"
#define TEXENV_COMBINE_PREVIOUS "34168"
#define TEXENV_COMBINE_SRC_COLOR "768"
#define TEXENV_COMBINE_ONE_MINUS_SRC_COLOR "769"
#define TEXENV_COMBINE_SRC_ALPHA "770"
#define TEXENV_COMBINE_ONE_MINUS_SRC_ALPHA "771"

#define ALPHA_TEST_FUNC_NEVER "512"
#define ALPHA_TEST_FUNC_LESS "513"
#define ALPHA_TEST_FUNC_EQUAL "514"
#define ALPHA_TEST_FUNC_LEQUAL "515"
#define ALPHA_TEST_FUNC_GREATER "516"
#define ALPHA_TEST_FUNC_NOTEQUAL "517"
#define ALPHA_TEST_FUNC_GEQUAL "518"
#define ALPHA_TEST_FUNC_ALWAYS "519"

#define FOG_MODE_LINEAR "9729"
#define FOG_MODE_EXP "2048"
#define FOG_MODE_EXP2 "2049"

typedef struct MaterialOptions {
    bool lighting;
    bool alphaTest;
    bool fog;
    bool pointSprite;
    bool transformVertex;
    uint8_t numTextures;
    ARBProgram* vertexProgram;
    ARBProgram* fragmentProgram;
} MaterialOptions;

typedef struct ShaderMaterial {
    GLuint program;

    struct {
        int attributes[VERTEX_ATTRIB_COUNT];
        int projectionMatrix;
        int modelViewMatrix;
        int textureMatrix;
        int alphaTest;
        int useTexture;
        int texture[MAX_TEXCOORDS];
        int texEnv[MAX_TEXCOORDS][6];
        int point[7];
        int fog[5];

        int lights[MAX_LIGHTS][7];
        int numLights;

        int materials[2][4];

        SparseArray programEnv;
        SparseArray programLocal;
    } location;
} ShaderMaterial;

extern ShaderMaterial* ShaderMaterial_create(MaterialOptions* options);
extern void ShaderMaterial_destroy(ShaderMaterial* material);
extern void ShaderMaterial_updatePointUniforms(ShaderMaterial* material, GLRenderer* renderer);
extern void ShaderMaterial_updateUniforms(ShaderMaterial* material, GLRenderer* renderer, MaterialOptions* options);

static inline uint32_t generateMaterialHash(MaterialOptions* options) {
    const char key[] = {0, 0, 0, 0, '-', 0, 0, 0, 0, '-', INT2CHR(options->lighting), '-', INT2CHR(options->alphaTest), '-', INT2CHR(options->fog), '-', INT2CHR(options->pointSprite), '-', INT2CHR(options->transformVertex), '-', INT2CHR(options->numTextures)};
    *(int*)(key+0) = options->vertexProgram ? options->vertexProgram->id : 0;
    *(int*)(key+5) = options->fragmentProgram ? options->fragmentProgram->id : 0;
    return fnv1aHash32(key, sizeof(key));
}

static inline void ShaderMaterial_getFogUniformLocations(GLuint program, int* locations) {
    char uniformName[64] = {0};
    const char* fogStructNames[] = {"color", "density", "start", "end", "scale"};
    for (int i = 0; i < ARRAY_SIZE(fogStructNames); i++) {
        sprintf(uniformName, "gd_Fog.%s", fogStructNames[i]);
        locations[i] = glGetUniformLocation(program, uniformName);
    }
}

static inline void ShaderMaterial_getPointUniformLocations(GLuint program, int* locations) {
    char uniformName[64] = {0};
    const char* pointStructNames[] = {"size", "sizeMin", "sizeMax", "fadeThresholdSize", "distanceConstantAttenuation", "distanceLinearAttenuation", "distanceQuadraticAttenuation"};
    for (int i = 0; i < ARRAY_SIZE(pointStructNames); i++) {
        sprintf(uniformName, "gd_Point.%s", pointStructNames[i]);
        locations[i] = glGetUniformLocation(program, uniformName);
    }
}

#endif