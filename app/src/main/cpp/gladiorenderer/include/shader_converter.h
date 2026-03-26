#ifndef GLADIO_SHADER_CONVERTER_H
#define GLADIO_SHADER_CONVERTER_H

#include "shader_material.h"
#include "shader_chunks.h"

#define COMPILE_STATUS_ERROR 0
#define COMPILE_STATUS_PENDING 1
#define COMPILE_STATUS_SUCCESS 2

typedef struct ShaderFunction {
    char* name;
    int lineStart;
    int lineEnd;
    bool hasBody;
    IntArray paramTypes;
} ShaderFunction;

typedef struct ShaderVariable {
    GLenum type;
    char* name;
    char* blockName;
    uint8_t typeQualifier;
    int location;
    int scopeId;
    int groupId;
    int arraySize;
    bool isMember;
    bool outOfScope;
    void* next;
    void* members;
    int lineStart;
} ShaderVariable;

typedef struct ShaderCode {
    ArrayList lines;
    ArrayMap variables;
    ArrayList functions;
    ArrayMap definedMacros;
    int scopeCount;

    ShaderFunction* lastFunction;
    ShaderVariable* lastVariable;
    ShaderVariable* lastInterfaceBlock;
    int groupCount;
    uint32_t flags;
    short version;
} ShaderCode;

typedef struct ShaderObject {
    GLuint id;
    GLenum type;
    ShaderCode code;
    bool attached;
    bool deleted;
    char compileStatus;
} ShaderObject;

typedef struct ShaderProgram {
    GLuint id;
    SparseArray fragDataLocations;
    ArrayList attachedShaders;
    bool hasBuiltinUniforms;
    bool hasBuiltinColor;

    struct {
        int attributes[VERTEX_ATTRIB_COUNT];
        int alphaTest;
        int projectionMatrix;
        int modelViewMatrix;
        int modelViewProjectionMatrix;
        int textureMatrix[MAX_TEXCOORDS];
        int fog[5];
    } location;
} ShaderProgram;

extern void ShaderConverter_setShaderSource(GLuint shaderId, GLsizei count, ArrayBuffer* inputBuffer);
extern void ShaderConverter_getShaderSource(ShaderObject* shader, ArrayBuffer* outSource);
extern GLuint ShaderConverter_createShader(GLenum type);
extern ShaderObject* ShaderConverter_getShader(GLuint shaderId);
extern void ShaderConverter_deleteShader(GLuint shaderId);
extern ShaderProgram* ShaderConverter_getProgram(GLuint programId);
extern void ShaderConverter_deleteProgram(GLuint programId);
extern GLuint ShaderConverter_createProgram();
extern void ShaderConverter_attachShader(GLuint programId, GLuint shaderId);
extern void ShaderConverter_detachShader(GLuint programId, GLuint shaderId);
extern void ShaderConverter_linkProgram(GLuint programId);
extern void ShaderConverter_getShaderiv(GLuint shaderId, GLenum pname, GLint* params);
extern void ShaderConverter_getProgramiv(GLuint target, GLenum pname, GLint* params);
extern void ShaderConverter_updateBoundProgram();
extern void ShaderConverter_onDestroy(GLClientState* clientState);

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
static inline void printShaderLines(GLenum type, GLuint shaderId, GLuint programId, char* shaderSource, int len) {
    println("================ SHADER INFO: %s:%u GL_PROGRAM:%u ================", glEnumToString(type), shaderId, programId);
    int lineNo = 1;
    FOREACH_LINE(shaderSource, len + 1, println("%d: %s", lineNo++, line););
}

static inline void debugShaderCode(GLenum shaderType, const char* shaderCode) {
    int shaderId = ShaderConverter_createShader(shaderType);

    ArrayBuffer stringBuf = {0};
    ArrayBuffer_putInt(&stringBuf, strlen(shaderCode));
    ArrayBuffer_putString(&stringBuf, shaderCode);
    ShaderConverter_setShaderSource(shaderId, 1, &stringBuf);
    ShaderObject* shader = ShaderConverter_getShader(shaderId);

    ArrayBuffer shaderSource = {0};
    ShaderConverter_getShaderSource(shader, &shaderSource);

    FOREACH_LINE(shaderSource.buffer, strlen(shaderSource.buffer) + 1, println("%s", line););
    exit(0);
}
#endif

#define MARK_VARIABLE_NAME(name) \
    char* bracket = strchr(name, '['); \
    char* dot = NULL;                 \
    do { \
        if (bracket) { \
            bracket[0] = '\0'; \
        } \
        else if (isalnum(name[0]) || name[0] == '_') { \
            dot = strchr(name, '.'); \
            if (dot) dot[0] = '\0'; \
        } \
    } \
    while (0)

#define UNMARK_VARIABLE_NAME(name) \
    do { \
        if (bracket) bracket[0] = '['; \
        if (dot) dot[0] = '.'; \
    } \
    while (0)

#define MARK_START_COMMENT(line) \
    char* comment = strstr(oldLine, "//"); \
    if (!comment) comment = strstr(oldLine, "/*"); \
    if (comment) comment[0] = '\0'

#define UNMARK_START_COMMENT() \
    if (comment) comment[0] = '/';

static inline int extractVariableArrayIndex(char* input) {
    char string[128];
    if (!substrv(input, '[', ']', string)) return -1;
    char* value = trim(string);
    return is_int(value) ? strtol(value, NULL, 10) : INT32_MAX;
}

#endif