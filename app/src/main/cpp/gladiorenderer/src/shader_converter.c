#include "shader_converter.h"
#include "string_utils.h"
#include "shader_material.h"
#include "gl_context.h"
#include "gl_renderer.h"

#define GL_VOID 0XFF01
#define GL_INTERFACE_BLOCK 0XFF02

#define TYPE_QUALIFIER_NONE 0
#define TYPE_QUALIFIER_CONST 1
#define TYPE_QUALIFIER_IN 2
#define TYPE_QUALIFIER_OUT 3
#define TYPE_QUALIFIER_UNIFORM 4
#define TYPE_QUALIFIER_ATTRIBUTE 5
#define TYPE_QUALIFIER_VARYING 6

#define FLAG_BUILTIN_COLOR (1<<0)
#define FLAG_BUILTIN_SECONDARY_COLOR (1<<1)
#define FLAG_BUILTIN_VERTEX (1<<2)
#define FLAG_BUILTIN_NORMAL (1<<3)
#define FLAG_BUILTIN_MULTITEXCOORD (1<<4)
#define FLAG_BUILTIN_FOGCOORD (1<<5)
#define FLAG_BUILTIN_MODEL_VIEW_MATRIX (1<<6)
#define FLAG_BUILTIN_PROJECTION_MATRIX (1<<7)
#define FLAG_BUILTIN_MODEL_VIEW_PROJECTION_MATRIX (1<<8)
#define FLAG_BUILTIN_TEXTURE_MATRIX (1<<9)
#define FLAG_BUILTIN_FRONT_COLOR (1<<10)
#define FLAG_BUILTIN_BACK_COLOR (1<<11)
#define FLAG_BUILTIN_FRONT_SECONDARY_COLOR (1<<12)
#define FLAG_BUILTIN_BACK_SECONDARY_COLOR (1<<13)
#define FLAG_BUILTIN_TEXCOORD (1<<14)
#define FLAG_BUILTIN_FOG_FRAGCOORD (1<<15)
#define FLAG_BUILTIN_FRAG_COLOR (1<<16)
#define FLAG_HAS_OPERATORS (1<<17)
#define FLAG_BUILTIN_FOG (1<<18)
#define FLAG_SHADER_EXTENSIONS (1<<19)

#define IS_GL_FLOAT_TYPE(type) (type == GL_FLOAT || type == GL_FLOAT_VEC2 || type == GL_FLOAT_VEC3 || type == GL_FLOAT_VEC4)
#define IS_GL_INT_TYPE(type) (type == GL_INT || type == GL_INT_VEC2 || type == GL_INT_VEC3 || type == GL_INT_VEC4)
#define IS_OPERATOR(chr) (chr == '=' || chr == '+' || chr == '-' || chr == '/' || chr == '*' || chr == '>' || chr == '<' || chr == '?' || chr == ':')

struct ReservedBuiltinWord {
    char* name;
    char* replace;
    uint32_t flags;
    GLenum shaderType;
};

struct ReservedWord {
    char* name;
    char* replace;
    GLenum shaderType;
};

typedef struct Subword {
    char* word;
    int start;
    int end;
    void* tag;
} Subword;

static struct ReservedBuiltinWord reservedBuiltinWords[] = {
    {"gl_Color", NULL, FLAG_BUILTIN_COLOR, GL_VERTEX_SHADER},
    {"gl_SecondaryColor", NULL, FLAG_BUILTIN_SECONDARY_COLOR, GL_VERTEX_SHADER},
    {"gl_Vertex", NULL, FLAG_BUILTIN_VERTEX, GL_VERTEX_SHADER},
    {"gl_Normal", NULL, FLAG_BUILTIN_NORMAL, GL_VERTEX_SHADER},
    {"gl_MultiTexCoord", NULL, FLAG_BUILTIN_MULTITEXCOORD, GL_VERTEX_SHADER},
    {"gl_FogCoord", NULL, FLAG_BUILTIN_FOGCOORD, GL_VERTEX_SHADER},
    {"gl_ModelViewMatrix", NULL, FLAG_BUILTIN_MODEL_VIEW_MATRIX, GL_VERTEX_SHADER},
    {"gl_ProjectionMatrix", NULL, FLAG_BUILTIN_PROJECTION_MATRIX, GL_VERTEX_SHADER},
    {"gl_ModelViewProjectionMatrix", NULL, FLAG_BUILTIN_MODEL_VIEW_PROJECTION_MATRIX, GL_VERTEX_SHADER},
    {"gl_TextureMatrix", NULL, FLAG_BUILTIN_TEXTURE_MATRIX, GL_VERTEX_SHADER},
    {"gl_FrontColor", NULL, FLAG_BUILTIN_FRONT_COLOR, GL_VERTEX_SHADER},
    {"gl_BackColor", NULL, FLAG_BUILTIN_BACK_COLOR, GL_VERTEX_SHADER},
    {"gl_FrontSecondaryColor", NULL, FLAG_BUILTIN_FRONT_SECONDARY_COLOR, GL_VERTEX_SHADER},
    {"gl_BackSecondaryColor", NULL, FLAG_BUILTIN_BACK_SECONDARY_COLOR, GL_VERTEX_SHADER},
    {"gl_TexCoord", NULL, FLAG_BUILTIN_TEXCOORD, GL_NONE},
    {"gl_Color", "(gl_FrontFacing ? gd_FrontColor : gd_BackColor)", (FLAG_BUILTIN_FRONT_COLOR | FLAG_BUILTIN_BACK_COLOR), GL_FRAGMENT_SHADER},
    {"gl_SecondaryColor", "(gl_FrontFacing ? gd_FrontSecondaryColor : gd_BackSecondaryColor)", (FLAG_BUILTIN_FRONT_SECONDARY_COLOR | FLAG_BUILTIN_BACK_SECONDARY_COLOR), GL_FRAGMENT_SHADER},
    {"gl_FragColor", NULL, FLAG_BUILTIN_FRAG_COLOR, GL_FRAGMENT_SHADER},
    {"gl_FragData[0]", "gd_FragColor", FLAG_BUILTIN_FRAG_COLOR, GL_FRAGMENT_SHADER},
    {"gl_FogFragCoord", NULL, FLAG_BUILTIN_FOG_FRAGCOORD, GL_NONE},
    {"gl_Fog", NULL, FLAG_BUILTIN_FOG, GL_NONE}
};

static struct ReservedWord reservedWords[] = {
    {"attribute", "in", GL_VERTEX_SHADER},
    {"varying", "out", GL_VERTEX_SHADER},
    {"varying", "in", GL_FRAGMENT_SHADER},
    {"sampler1D", "sampler2D", GL_FRAGMENT_SHADER},
    {"texture2D", "texture", GL_FRAGMENT_SHADER},
    {"texture2DLod", "textureLod", GL_FRAGMENT_SHADER},
    {"texture2DGradARB", "textureGrad", GL_FRAGMENT_SHADER},
    {"texture2DLodOffset", "textureLodOffset", GL_FRAGMENT_SHADER},
    {"texture3D", "texture", GL_FRAGMENT_SHADER},
    {"filter", "gd_Filter", GL_FRAGMENT_SHADER}
};

static char* allowedExtensions[] = {"GL_ARB_shader_texture_lod"};

static bool startsWithPreprocessor(char* name, char* line) {
    return line[0] == '#' && cstartswith(name, ltrim(line+1));
}

static bool isFunctionName(char* line, int start, int end) {
    char oldChar = line[end];
    line[end] = '\0';
    bool invalid = strchr(line+start, '.') || strchr(line+start, '[');
    line[end] = oldChar;
    return !invalid && *ltrim(line+end) == '(';
}

static uint8_t getGLTypeComponentCount(GLenum type) {
    switch (type) {
        case GL_FLOAT_VEC2:
        case GL_INT_VEC2:
            return 2;
        case GL_FLOAT_VEC3:
        case GL_INT_VEC3:
            return 3;
        case GL_FLOAT_VEC4:
        case GL_INT_VEC4:
            return 4;
        default:
            return 1;
    }
}

static int getTypeQualifierAsEnum(char* name) {
    if (strcmp(name, "const") == 0) return TYPE_QUALIFIER_CONST;
    else if (strcmp(name, "in") == 0) return TYPE_QUALIFIER_IN;
    else if (strcmp(name, "out") == 0) return TYPE_QUALIFIER_OUT;
    else if (strcmp(name, "uniform") == 0) return TYPE_QUALIFIER_UNIFORM;
    else if (strcmp(name, "attribute") == 0) return TYPE_QUALIFIER_ATTRIBUTE;
    else if (strcmp(name, "varying") == 0) return TYPE_QUALIFIER_VARYING;
    return 0;
}

static const char* getTypeQualifierAsString(uint8_t qualifier) {
    switch (qualifier) {
        case TYPE_QUALIFIER_CONST: return "const";
        case TYPE_QUALIFIER_IN: return "in";
        case TYPE_QUALIFIER_OUT: return "out";
        case TYPE_QUALIFIER_UNIFORM: return "uniform";
        case TYPE_QUALIFIER_ATTRIBUTE: return "attribute";
        case TYPE_QUALIFIER_VARYING: return "varying";
        default:
            return "";
    }
}

static GLenum getGLTypeAsEnum(char* name) {
    if (strcmp(name, "int") == 0) return GL_INT;
    else if (strcmp(name, "bool") == 0) return GL_BOOL;
    else if (strcmp(name, "float") == 0) return GL_FLOAT;
    else if (strcmp(name, "vec2") == 0) return GL_FLOAT_VEC2;
    else if (strcmp(name, "vec3") == 0) return GL_FLOAT_VEC3;
    else if (strcmp(name, "vec4") == 0) return GL_FLOAT_VEC4;
    else if (strcmp(name, "ivec2") == 0) return GL_INT_VEC2;
    else if (strcmp(name, "ivec3") == 0) return GL_INT_VEC3;
    else if (strcmp(name, "ivec4") == 0) return GL_INT_VEC4;
    else if (strcmp(name, "bvec2") == 0) return GL_BOOL_VEC2;
    else if (strcmp(name, "bvec3") == 0) return GL_BOOL_VEC3;
    else if (strcmp(name, "bvec4") == 0) return GL_BOOL_VEC4;
    else if (strcmp(name, "mat2") == 0) return GL_FLOAT_MAT2;
    else if (strcmp(name, "mat3") == 0) return GL_FLOAT_MAT3;
    else if (strcmp(name, "mat4") == 0) return GL_FLOAT_MAT4;
    else if (strcmp(name, "sampler1D") == 0 ||
             strcmp(name, "sampler2D") == 0) return GL_SAMPLER_2D;
    else if (strcmp(name, "sampler3D") == 0) return GL_SAMPLER_3D;
    else if (strcmp(name, "sampler2DShadow") == 0) return GL_SAMPLER_2D_SHADOW;
    else if (strcmp(name, "samplerCube") == 0) return GL_SAMPLER_CUBE;
    else if (strcmp(name, "samplerCubeShadow") == 0) return GL_SAMPLER_CUBE_SHADOW;
    else if (strcmp(name, "void") == 0) return GL_VOID;
    return GL_NONE;
}

static const char* getGLTypeAsString(GLenum type) {
    switch (type) {
        case GL_INT: return "int";
        case GL_BOOL: return "bool";
        case GL_FLOAT: return "float";
        case GL_FLOAT_VEC2: return "vec2";
        case GL_FLOAT_VEC3: return "vec3";
        case GL_FLOAT_VEC4: return "vec4";
        case GL_INT_VEC2: return "ivec2";
        case GL_INT_VEC3: return "ivec3";
        case GL_INT_VEC4: return "ivec4";
        case GL_BOOL_VEC2: return "bvec2";
        case GL_BOOL_VEC3: return "bvec3";
        case GL_BOOL_VEC4: return "bvec4";
        case GL_FLOAT_MAT2: return "mat2";
        case GL_FLOAT_MAT3: return "mat3";
        case GL_FLOAT_MAT4: return "mat4";
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D: return "sampler2D";
        case GL_SAMPLER_3D: return "sampler3D";
        case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
        case GL_SAMPLER_CUBE: return "samplerCube";
        case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow";
        case GL_VOID: return "void";
        default: return NULL;
    }
}

static char* stringifyShaderVariable(ShaderVariable* variable) {
    char result[1024] = {0};
    if (variable->type == GL_INTERFACE_BLOCK) {
        strcat(result, getTypeQualifierAsString(variable->typeQualifier));
        strcat(result, " ");
        strcat(result, variable->blockName);
        strcat(result, " {");

        ShaderVariable* member = variable->members;
        while (member) {
            strcat(result, " ");
            char* string = stringifyShaderVariable(member);
            strcat(result, string);
            free(string);
            member = member->members;
        }

        strcat(result, " }");
    }
    else {
        if (variable->typeQualifier != TYPE_QUALIFIER_NONE) {
            strcat(result, getTypeQualifierAsString(variable->typeQualifier));
            strcat(result, " ");
        }

        strcat(result, getGLTypeAsString(variable->type));
    }

    strcat(result, " ");
    strcat(result, variable->name);
    if (variable->arraySize > 0) {
        char value[8];
        sprintf(value, "%d", variable->arraySize);
        strcat(result, "[");
        strcat(result, value);
        strcat(result, "]");
    }
    strcat(result, ";");
    return strdup(result);
}

static void extractDefinedMacro(ShaderCode* shaderCode, char* line) {
    if (!startsWithPreprocessor("define", line)) return;

    char* chr = line + 7;
    int nameStart = -1;
    char* macroName = NULL;
    char* macroValue = NULL;

    while (1) {
        if (isalnum(*chr) || *chr == '_' || *chr == '.') {
            if (nameStart == -1) nameStart = chr - line;
        }
        else if (nameStart != -1) {
            int len = chr - line - nameStart;
            if (!macroName) {
                if (*chr == ' ') {
                    macroName = substr(line, nameStart, len);
                }
                else if (*chr == '(') break;
            }
            else {
                macroValue = substr(line, nameStart, len);
                break;
            }
            nameStart = -1;
        }

        if (!*chr) break;
        chr++;
    }

    if (macroName && macroValue) {
        ArrayMap_put(&shaderCode->definedMacros, macroName, macroValue);
    }
    else {
        MEMFREE(macroName);
        MEMFREE(macroValue);
    }
}

static void extractLayoutQualifiers(char* line, ArrayMap* result) {
    char* chr = line;
    bool started = false;
    int nameStart = -1;
    int nameEnd = -1;
    char* qualifierName = NULL;
    char* qualifierValue = NULL;

    while (*chr) {
        if (*chr == '(') {
            started = true;
        }
        else if (started) {
            if (isalnum(*chr) || *chr == '_') {
                if (nameStart == -1) nameStart = chr - line;
            }
            else if (nameStart != -1 && nameEnd == -1) {
                nameEnd = chr - line;
            }
        }

        if ((*chr == ',' || *chr == '=' || *chr == ')') && nameStart != -1 && nameEnd != -1) {
            if (*chr == '=') {
                qualifierName = substr(line, nameStart, nameEnd-nameStart);
            }
            else if (*chr == ',' || *chr == ')') {
                qualifierValue = substr(line, nameStart, nameEnd-nameStart);
            }

            if (qualifierName && qualifierValue) {
                ArrayMap_put(result, qualifierName, qualifierValue);
                qualifierName = NULL;
                qualifierValue = NULL;
            }
            else if (qualifierValue) {
                ArrayMap_put(result, qualifierValue, NULL);
                qualifierValue = NULL;
            }

            nameStart = -1;
            nameEnd = -1;
        }

        if (started && *chr == ')') break;
        chr++;
    }

    MEMFREE(qualifierName);
    MEMFREE(qualifierValue);
}

static int extractShaderFunctionParams(char* line, IntArray* declaredTypes, IntArray* ranges) {
    char* chr = line;
    int groupCount = 0;
    bool started = false;
    int nameStart = -1;
    int paramsCount = 0;

    while (*chr) {
        if (*chr == '(') {
            started = true;
            groupCount++;
            if (groupCount > 1 && nameStart == -1) nameStart = chr - line;
        }
        else if (*chr == ')') {
            groupCount--;
        }
        else if (groupCount == 1 && nameStart == -1 && !isspace(*chr)) {
            nameStart = chr - line;
        }

        if (((*chr == ',' && groupCount == 1) || (*chr == ')' && groupCount == 0)) && nameStart != -1) {
            if (declaredTypes) {
                GLenum paramType = GL_NONE;
                char* start = line + nameStart;
                do {
                    char* end;
                    start = strwrd(start, NULL, &end);
                    if (!start) break;
                    char old = end[0];
                    end[0] = '\0';
                    paramType = getGLTypeAsEnum(start);
                    end[0] = old;
                    start = end;
                }
                while (paramType == GL_NONE);
                IntArray_add(declaredTypes, paramType);
            }
            if (ranges) {
                int nameEnd = chr - line;
                while (isspace(line[nameEnd-1])) nameEnd--;
                IntArray_add(ranges, nameStart);
                IntArray_add(ranges, nameEnd);
            }

            paramsCount++;
            nameStart = -1;
        }

        if (started && *chr == ')' && groupCount == 0) break;
        chr++;
    }

    return paramsCount;
}

static void extractShaderDataTypes(ShaderCode* shaderCode, char* line) {
    char* chr = line;
    int nameStart = -1;
    GLenum componentType = GL_NONE;
    uint8_t typeQualifier = TYPE_QUALIFIER_NONE;
    int location = -1;
    bool commaStarted = false;

    while (1) {
        if (*chr == '{') {
            if (shaderCode->lastInterfaceBlock) {
                shaderCode->lastInterfaceBlock->outOfScope = false;
                typeQualifier = TYPE_QUALIFIER_NONE;
                location = -1;
            }

            shaderCode->scopeCount++;
            if (shaderCode->lastFunction && !shaderCode->lastFunction->hasBody && shaderCode->scopeCount == 1) {
                shaderCode->lastFunction->hasBody = true;
            }
        }
        else if (*chr == '}' && shaderCode->scopeCount > 0) {
            if (shaderCode->lastInterfaceBlock && !shaderCode->lastInterfaceBlock->outOfScope) {
                shaderCode->lastVariable = shaderCode->lastInterfaceBlock;
                shaderCode->lastInterfaceBlock = NULL;
            }
            else {
                for (int i = 0; i < shaderCode->variables.size; i++) {
                    ShaderVariable* variable = shaderCode->variables.entries[i].value;
                    if (variable->scopeId == shaderCode->scopeCount) variable->outOfScope = true;
                }
            }
            shaderCode->scopeCount--;

            if (shaderCode->lastFunction && shaderCode->scopeCount == 0 && shaderCode->lastFunction->hasBody) {
                shaderCode->lastFunction->lineEnd = shaderCode->lines.size;
                ArrayList_add(&shaderCode->functions, shaderCode->lastFunction);
                shaderCode->lastFunction = NULL;
            }
        }

        if (shaderCode->lastFunction) {
            if (*chr == '(') {
                shaderCode->groupCount++;
            }
            else if (*chr == ')' && shaderCode->groupCount > 0) {
                shaderCode->groupCount--;
            }
            else if (*chr == ';' && shaderCode->groupCount == 0 && !shaderCode->lastFunction->hasBody) {
                shaderCode->lastFunction->lineEnd = shaderCode->lines.size;
                ArrayList_add(&shaderCode->functions, shaderCode->lastFunction);
                shaderCode->lastFunction = NULL;
            }
        }

        if (shaderCode->lastVariable) {
            bool addVariable = false;

            if (*chr == ';') {
                if (shaderCode->lastVariable->type == GL_INTERFACE_BLOCK) {
                    if (!shaderCode->lastVariable->name && nameStart != -1) {
                        int nameEnd = chr - line;
                        shaderCode->lastVariable->name = substr(line, nameStart, nameEnd-nameStart);
                    }

                    if (!shaderCode->lastVariable->name) shaderCode->lastVariable->name = strdup(shaderCode->lastVariable->blockName);
                    ArrayMap_put(&shaderCode->variables, shaderCode->lastVariable->name, shaderCode->lastVariable);
                    shaderCode->lastVariable = NULL;
                }
                else {
                    if (commaStarted) {
                        addVariable = true;
                        commaStarted = false;
                    }
                    else shaderCode->lastVariable = NULL;
                }
            }
            else if (*chr == '=') {
                shaderCode->lastVariable = NULL;
                commaStarted = false;
            }
            else if (*chr == ',') {
                if (commaStarted) addVariable = true;
                else shaderCode->lastVariable = NULL;
            }
            else if (shaderCode->lastVariable->type == GL_INTERFACE_BLOCK && *chr == '[') {
                if (!shaderCode->lastVariable->name && nameStart != -1) {
                    int nameEnd = chr - line;
                    shaderCode->lastVariable->name = substr(line, nameStart, nameEnd-nameStart);
                }

                int arraySize = extractVariableArrayIndex(chr);
                if (arraySize > 0) shaderCode->lastVariable->arraySize = arraySize;
            }

            if (addVariable && nameStart != -1) {
                int nameEnd = chr - line;
                ShaderVariable* variable = memdup(shaderCode->lastVariable, sizeof(ShaderVariable));
                variable->name = substr(line, nameStart, nameEnd-nameStart);
                variable->next = NULL;
                shaderCode->lastVariable->next = variable;
                shaderCode->lastVariable = variable;
                ArrayMap_put(&shaderCode->variables, variable->name, variable);
                nameStart = -1;
            }

            if (*chr == ';') shaderCode->lastVariable = NULL;
        }

        if (isalnum(*chr) || *chr == '_' || *chr == '#') {
            if (nameStart == -1) nameStart = chr - line;
        }
        else if (nameStart != -1) {
            int nameEnd = chr - line;
            char* name = line + nameStart;

            if (startsWithPreprocessor("define", name)) break;

            if (shaderCode->lastInterfaceBlock && shaderCode->lastInterfaceBlock->outOfScope) {
                MEMFREE(shaderCode->lastInterfaceBlock->blockName);
                MEMFREE(shaderCode->lastInterfaceBlock);
            }

            if (componentType != GL_NONE) {
                if (!isdigit(name[0])) {
                    if (*ltrim(chr) == '(') {
                        if (!shaderCode->lastFunction) {
                            ShaderFunction* function = calloc(1, sizeof(ShaderFunction));
                            function->name = substr(line, nameStart, nameEnd-nameStart);
                            function->lineStart = shaderCode->lines.size;
                            shaderCode->lastFunction = function;
                            shaderCode->groupCount++;
                            extractShaderFunctionParams(name, &function->paramTypes, NULL);
                        }
                    }
                    else if (!shaderCode->lastVariable) {
                        int scopeId = shaderCode->scopeCount == 0 && shaderCode->lastFunction ? 1 : shaderCode->scopeCount;
                        ShaderVariable* variable = calloc(1, sizeof(ShaderVariable));
                        variable->name = substr(line, nameStart, nameEnd-nameStart);
                        variable->type = componentType;
                        variable->scopeId = scopeId;
                        variable->groupId = shaderCode->groupCount;
                        variable->typeQualifier = typeQualifier;
                        variable->location = location;
                        variable->lineStart = shaderCode->lines.size;

                        if (shaderCode->lastInterfaceBlock && !shaderCode->lastInterfaceBlock->outOfScope) {
                            ShaderVariable* last = shaderCode->lastInterfaceBlock;
                            while (last->members) last = last->members;
                            last->members = variable;
                            variable->isMember = true;
                        }

                        if (*chr == '[') {
                            int arraySize = extractVariableArrayIndex(chr);
                            if (arraySize > 0) variable->arraySize = arraySize;
                        }
                        else if (shaderCode->groupCount == 0 && *ltrim(chr) == ',') {
                            shaderCode->lastVariable = variable;
                            commaStarted = true;
                        }

                        if (!variable->isMember) ArrayMap_put(&shaderCode->variables, variable->name, variable);
                    }
                }

                componentType = GL_NONE;
                typeQualifier = TYPE_QUALIFIER_NONE;
                location = -1;
            }
            else {
                if (cstartswith("layout", name) && *ltrim(chr) == '(') {
                    ArrayMap layoutQualifiers = {0};
                    extractLayoutQualifiers(chr, &layoutQualifiers);
                    char* value = ArrayMap_get(&layoutQualifiers, "location");
                    if (value && is_int(value)) location = strtol(value, NULL, 10);
                    ArrayMap_free(&layoutQualifiers, true, true);
                }
                else if (*chr == ' ') {
                    char oldChar = line[nameEnd];
                    line[nameEnd] = '\0';

                    int enumValue = 0;
                    if ((enumValue = getGLTypeAsEnum(name)) != GL_NONE) {
                        componentType = enumValue;
                    }
                    else if (componentType == GL_NONE && (
                             typeQualifier == TYPE_QUALIFIER_IN ||
                             typeQualifier == TYPE_QUALIFIER_OUT ||
                             typeQualifier == TYPE_QUALIFIER_UNIFORM)) {
                        ShaderVariable* variable = calloc(1, sizeof(ShaderVariable));
                        variable->blockName = strdup(name);
                        variable->type = GL_INTERFACE_BLOCK;
                        variable->typeQualifier = typeQualifier;
                        variable->location = location;
                        variable->lineStart = shaderCode->lines.size;
                        variable->outOfScope = true;
                        shaderCode->lastInterfaceBlock = variable;
                    }
                    else if ((enumValue = getTypeQualifierAsEnum(name)) != TYPE_QUALIFIER_NONE) {
                        typeQualifier = enumValue;
                    }

                    line[nameEnd] = oldChar;
                }
            }

            nameStart = -1;
        }

        if (*chr == '\0') break;
        chr++;
    }
}

static char* implicitConvertIntToFloat(ShaderCode* shaderCode, char* line) {
    int i;
    int wordStart = -1;
    int wordEnd = -1;
    bool operatorFound = 0;
    bool groupStarted = false;
    int bracketCount = 0;

    ArrayList subwords = {0};
    bool hasOperator = false;

    i = 0;
    while (1) {
        char chr = line[i];

        if (chr == '[') {
            if (bracketCount == 0) {
                wordStart = i + 1;
                hasOperator = false;
            }
            bracketCount++;
        }
        else if (chr == ']') {
            bracketCount--;
            if (bracketCount == 0 && hasOperator) {
                Subword* subword = calloc(1, sizeof(Subword));
                subword->word = substr(line, wordStart, i - wordStart);
                subword->start = wordStart;
                subword->end = i;
                ArrayList_add(&subwords, subword);
                wordStart = -1;
            }
        }
        else if (bracketCount > 0 && IS_OPERATOR(chr)) {
            hasOperator = true;
        }

        if (chr == '\0') break;
        i++;
    }

    if (subwords.size > 0) {
        for (i = subwords.size-1; i >= 0; i--) {
            Subword* subword = subwords.elements[i];
            subword->word = implicitConvertIntToFloat(shaderCode, subword->word);
            line = substr_replace(subword->word, subword->start, subword->end, line);
            free(subword->word);
        }
        ArrayList_free(&subwords, true);
    }

    wordStart = -1;
    bracketCount = 0;

    i = 0;
    while (1) {
        char chr = line[i];

        if (chr == '[') bracketCount++;
        else if (chr == '(') groupStarted = true;
        else if (chr == ')') groupStarted = false;

        if (bracketCount > 0 && chr != ']') goto next;
        bool isSciNotation = wordStart != -1 && (chr == '-' || chr == '+') && (line[i-1] == 'e' || line[i-1] == 'E') && isdigit(line[wordStart]);

        if (isalnum(chr) || chr == '_' || chr == '.' || bracketCount > 0 || isSciNotation) {
            if (wordStart == -1) wordStart = i;
        }
        else if (wordStart != -1 && wordEnd == -1) {
            wordEnd = i;
            bool isFunction = isFunctionName(line, wordStart, wordEnd);

            if ((operatorFound || groupStarted) && !isFunction && (isalnum(line[wordStart]) || line[wordStart] == '_')) {
                Subword* subword = calloc(1, sizeof(Subword));
                subword->word = substr(line, wordStart, wordEnd - wordStart);
                subword->start = wordStart;
                subword->end = wordEnd;
                ArrayList_add(&subwords, subword);

                if (groupStarted) {
                    wordStart = -1;
                    wordEnd = -1;
                }
            }

            char* name = line + wordStart;
            if (cstartswith("float", name) ||
                cstartswith("vec2", name) ||
                cstartswith("vec3", name) ||
                cstartswith("vec4", name) || operatorFound) {
                wordStart = -1;
                wordEnd = -1;
                operatorFound = false;
            }

            groupStarted = false;
        }

        if (chr == ']') bracketCount--;

        bool hasWord = wordStart != -1 && wordEnd != -1;
        if ((hasWord && (chr == '(' || chr == ')' || chr == ',' || chr == ';' ||
                         strncmp(line + wordStart, "return", wordEnd - wordStart) == 0))) {
            wordStart = -1;
            wordEnd = -1;
            operatorFound = false;
            hasWord = false;
        }

        if (IS_OPERATOR(chr)) {
            bool skip = (!hasWord && groupStarted) || isSciNotation;
            if ((chr == '+' || chr == '-') && wordStart != -1) {
                char* prev = &line[i];
                while (isspace(*--prev));
                char* next = &line[i];
                while (isspace(*++next));
                if (*prev == '+' || *prev == '-' ||
                    *next == '+' || *next == '-') skip = true;
            }

            if (!skip) {
                if (hasWord) {
                    Subword* subword = calloc(1, sizeof(Subword));
                    subword->word = substr(line, wordStart, wordEnd - wordStart);
                    subword->start = wordStart;
                    subword->end = wordEnd;
                    ArrayList_add(&subwords, subword);
                }

                operatorFound = i != wordStart;
                wordStart = -1;
                wordEnd = -1;
            }
        }

        next:
        if (chr == '\0') break;
        i++;
    }

    bool convert = false;
    for (i = subwords.size-1; i >= 0; i--) {
        Subword* subword = subwords.elements[i];
        char* name = subword->word;
        char* macroValue = ArrayMap_get(&shaderCode->definedMacros, subword->word);
        if (macroValue) name = macroValue;
        MARK_VARIABLE_NAME(name);

        while (*name == '+' || *name == '-') name++;
        char* operatorAdd = strchr(name, '+');
        if (operatorAdd) operatorAdd[0] = '\0';
        char* operatorSub = strchr(name, '-');
        if (operatorSub) operatorSub[0] = '\0';

        ShaderVariable* variable = ArrayMap_get(&shaderCode->variables, name);
        if (variable && IS_GL_FLOAT_TYPE(variable->type)) convert = true;
        subword->tag = variable;
        UNMARK_VARIABLE_NAME(name);
        if (operatorAdd) operatorAdd[0] = '+';
        if (operatorSub) operatorSub[0] = '-';
    }

    char* oldLine;
    for (i = subwords.size-1; i >= 0; i--) {
        Subword* subword = subwords.elements[i];

        if (convert) {
            char* macroValue = ArrayMap_get(&shaderCode->definedMacros, subword->word);
            char* value = macroValue ? macroValue : subword->word;

            if (is_int(value)) {
                char replace[8];
                sprintf(replace, "%s.0", value);
                line = substr_replace(replace, subword->start, subword->end, oldLine = line);
                free(oldLine);
            }
            else {
                ShaderVariable* variable = subword->tag;
                if (variable && IS_GL_INT_TYPE(variable->type)) {
                    char componentCount = getGLTypeComponentCount(variable->type);

                    char* bracket = strrchr(subword->word, ']');
                    char* dot = strchr(subword->word + (bracket ? bracket-subword->word : 0), '.');
                    if (dot) componentCount = strlen(dot + 1);

                    char replace[64];
                    if (componentCount > 1) {
                        sprintf(replace, "vec%d(%s)", componentCount, subword->word);
                    }
                    else sprintf(replace, "float(%s)", subword->word);
                    line = substr_replace(replace, subword->start, subword->end, oldLine = line);
                    free(oldLine);
                }
            }
        }

        free(subword->word);
    }

    ArrayList_free(&subwords, true);
    return line;
}

static void checkBuiltinTextureFunctionParams(ShaderCode* shaderCode, char* line, int* index, uint8_t minComponentCount, IntArray* ranges, ArrayList* subwords) {
    int offset = *index;
    int paramCount = ranges->size / 2;

    if (paramCount >= 2) {
        int start = ranges->values[1*2+0] + offset;
        int end = ranges->values[1*2+1] + offset;
        char* name = line + start;

        char oldChar = line[end];
        line[end] = '\0';

        uint8_t componentCount = 0;
        char* dot = strchr(name, '.');
        if (dot) {
            componentCount = strlen(dot + 1);
        }
        else {
            ShaderVariable* variable = ArrayMap_get(&shaderCode->variables, name);
            if (variable) componentCount = getGLTypeComponentCount(variable->type);
        }

        if (componentCount != 0 && componentCount < minComponentCount) {
            char text[256];
            sprintf(text, "vec%d(%s, 0.0)", minComponentCount, name);

            Subword* subword = calloc(1, sizeof(Subword));
            subword->word = strdup(text);
            subword->start = start;
            subword->end = end;
            ArrayList_add(subwords, subword);
            *index = end;
        }
        line[end] = oldChar;
    }

    if (paramCount >= 3) {
        int start = ranges->values[2*2+0] + offset;
        int end = ranges->values[2*2+1] + offset;
        char* name = line + start;

        char oldChar = line[end];
        line[end] = '\0';

        char text[256];
        sprintf(text, "float(%s)", name);
        line[end] = oldChar;

        Subword* subword = calloc(1, sizeof(Subword));
        subword->word = strdup(text);
        subword->start = start;
        subword->end = end;
        ArrayList_add(subwords, subword);
        *index = end;
    }
}

static void checkBuiltinMathFunctionIntParams(ShaderCode* shaderCode, char* line, int* index, ArrayList* subwords) {
    int offset = *index;
    IntArray ranges = {0};
    extractShaderFunctionParams(line + offset, NULL, &ranges);
    int paramCount = ranges.size / 2;
    bool convert = false;

    for (int i = 0; i < paramCount && !convert; i++) {
        int start = ranges.values[i*2+0] + offset;
        int end = ranges.values[i*2+1] + offset;
        char* name = line + start;
        char oldChar = line[end];
        line[end] = '\0';
        convert = !is_int(name) && !cstartswith("int(", name);
        line[end] = oldChar;
    }

    if (convert) {
        for (int i = 0; i < paramCount; i++) {
            int start = ranges.values[i*2+0] + offset;
            int end = ranges.values[i*2+1] + offset;
            char* name = line + start;
            char oldChar = line[end];
            line[end] = '\0';

            if (is_int(name)) {
                char text[32];
                sprintf(text, "%s.0", name);

                Subword* subword = calloc(1, sizeof(Subword));
                subword->word = strdup(text);
                subword->start = start;
                subword->end = end;
                ArrayList_add(subwords, subword);
                *index = MAX(*index, end);
            }

            line[end] = oldChar;
        }
    }

    IntArray_clear(&ranges);
}

static void checkDeclaredFunctionIntParams(ShaderCode* shaderCode,  char* functionName, char* line, int* index, ArrayList* subwords) {
    int offset = *index;
    int lineIndex = shaderCode->lines.size;
    for (int i = 0; i < shaderCode->functions.size; i++) {
        ShaderFunction* function = shaderCode->functions.elements[i];
        if (lineIndex <= function->lineStart || lineIndex <= function->lineEnd) continue;

        char oldChar = line[offset];
        line[offset] = '\0';
        bool found = strcmp(function->name, functionName) == 0;
        line[offset] = oldChar;
        if (!found) continue;

        IntArray ranges = {0};
        extractShaderFunctionParams(line + offset, NULL, &ranges);

        int paramCount = ranges.size / 2;
        if (paramCount == function->paramTypes.size) {
            for (int j = 0; j < paramCount; j++) {
                int start = ranges.values[j*2+0] + offset;
                int end = ranges.values[j*2+1] + offset;
                char* name = line + start;
                oldChar = line[end];

                line[end] = '\0';
                if (function->paramTypes.values[j] == GL_FLOAT && is_int(name)) {
                    char text[32];
                    sprintf(text, "%s.0", name);

                    Subword* subword = calloc(1, sizeof(Subword));
                    subword->word = strdup(text);
                    subword->start = start;
                    subword->end = end;
                    ArrayList_add(subwords, subword);
                    *index = MAX(*index, end);
                }
                line[end] = oldChar;
            }
        }

        IntArray_clear(&ranges);
    }
}

static char* implicitConvertFunctionParams(ShaderCode* shaderCode, char* line) {
    ArrayList subwords = {0};
    int i = 0;
    int nameStart = -1;
    int groupStart = -1;
    int groupCount = 0;

    i = 0;
    while (1) {
        char chr = line[i];

        if (chr == '(') {
            groupCount++;
        }
        else if (chr == ')') {
            groupCount--;
            if (groupCount == 0 && groupStart != -1) {
                Subword* subword = calloc(1, sizeof(Subword));
                subword->word = substr(line, groupStart, i - groupStart);
                subword->start = groupStart;
                subword->end = i;
                ArrayList_add(&subwords, subword);
                groupStart = -1;
            }
        }

        if (isalnum(chr) || chr == '_') {
            if (nameStart == -1) nameStart = i;
        }
        else if (nameStart != -1) {
            if (isFunctionName(line, nameStart, i) && groupStart == -1) groupStart = i + 1;
            nameStart = -1;
        }

        if (chr == '\0') break;
        i++;
    }

    if (subwords.size > 0) {
        for (i = subwords.size-1; i >= 0; i--) {
            Subword* subword = subwords.elements[i];
            subword->word = implicitConvertFunctionParams(shaderCode, subword->word);
            line = substr_replace(subword->word, subword->start, subword->end, line);
            free(subword->word);
        }
        ArrayList_free(&subwords, true);
    }

    i = 0;
    nameStart = -1;
    while (1) {
        char chr = line[i];

        if (isalnum(chr) || chr == '_') {
            if (nameStart == -1) nameStart = i;
        }
        else if (nameStart != -1) {
            if (isFunctionName(line, nameStart, i)) {
                char oldChar = line[i];
                line[i] = '\0';
                char* name = line + nameStart;
                bool isBuiltinMathFunction = strcmp(name, "min") == 0 ||
                                             strcmp(name, "max") == 0 ||
                                             strcmp(name, "pow") == 0 ||
                                             strcmp(name, "mod") == 0 ||
                                             strcmp(name, "clamp") == 0 ||
                                             strcmp(name, "smoothstep") == 0;
                bool isBuiltinTextureFunction = !isBuiltinMathFunction && (strcmp(name, "texture") == 0 ||
                                                                           strcmp(name, "textureLod") == 0 ||
                                                                           strcmp(name, "textureProj") == 0);
                line[i] = oldChar;

                if (isBuiltinMathFunction) {
                    checkBuiltinMathFunctionIntParams(shaderCode, line, &i, &subwords);
                }
                else if (isBuiltinTextureFunction) {
                    if (cstartswith("texture(", name) || cstartswith("textureLod(", name)) {
                        IntArray ranges = {0};
                        extractShaderFunctionParams(line + i, NULL, &ranges);
                        checkBuiltinTextureFunctionParams(shaderCode, line, &i, 2, &ranges, &subwords);
                        IntArray_clear(&ranges);
                    }
                    else if (cstartswith("textureProj(", name)) {
                        IntArray ranges = {0};
                        extractShaderFunctionParams(line + i, NULL, &ranges);
                        checkBuiltinTextureFunctionParams(shaderCode, line, &i, 3, &ranges, &subwords);
                        IntArray_clear(&ranges);
                    }
                }
                else checkDeclaredFunctionIntParams(shaderCode, name, line, &i, &subwords);
            }

            nameStart = -1;
        }

        if (chr == '\0') break;
        i++;
    }

    char* oldLine;
    for (i = subwords.size-1; i >= 0; i--) {
        Subword* subword = subwords.elements[i];
        line = substr_replace(subword->word, subword->start, subword->end, oldLine = line);
        free(oldLine);
        free(subword->word);
    }

    ArrayList_free(&subwords, true);
    return line;
}

static char* replaceReservedWords(ShaderObject* shader, char* line) {
    ArrayList subwords = {0};
    int i = 0;
    int j = 0;
    int nameStart = -1;
    BITMASK_UNSET(shader->code.flags, FLAG_HAS_OPERATORS);

    while (1) {
        char chr = line[i];
        if (IS_OPERATOR(chr)) BITMASK_SET(shader->code.flags, FLAG_HAS_OPERATORS);

        if (isalnum(chr) || chr == '_') {
            if (nameStart == -1) nameStart = i;
        }
        else if (nameStart != -1) {
            char* name = line + nameStart;
            int len = i - nameStart;
            char* replace = NULL;

            if (len > 3 && name[0] == 'g' && name[1] == 'l' && name[2] == '_') {
                for (j = 0; j < ARRAY_SIZE(reservedBuiltinWords); j++) {
                    if ((reservedBuiltinWords[j].shaderType == GL_NONE || reservedBuiltinWords[j].shaderType == shader->type) && cstartswith(reservedBuiltinWords[j].name, name)) {
                        if (len < strlen(reservedBuiltinWords[j].name)) i = nameStart + strlen(reservedBuiltinWords[j].name);
                        if (reservedBuiltinWords[j].replace) {
                            replace = strdup(reservedBuiltinWords[j].replace);
                        }
                        else name[1] = 'd';
                        if (reservedBuiltinWords[j].flags != 0) BITMASK_SET(shader->code.flags, reservedBuiltinWords[j].flags);
                        break;
                    }
                }
            }
            else {
                char oldChar = line[i];
                line[i] = '\0';
                for (j = 0; j < ARRAY_SIZE(reservedWords); j++) {
                    if ((reservedWords[j].shaderType == GL_NONE || reservedWords[j].shaderType == shader->type) && strcmp(reservedWords[j].name, name) == 0) {
                        replace = strdup(reservedWords[j].replace);
                        break;
                    }
                }
                line[i] = oldChar;
            }

            if (replace) {
                Subword* subword = calloc(1, sizeof(Subword));
                subword->word = replace;
                subword->start = nameStart;
                subword->end = i;
                ArrayList_add(&subwords, subword);
            }

            nameStart = -1;
        }

        if (chr == '\0') break;
        i++;
    }

    char* oldLine;
    for (i = subwords.size-1; i >= 0; i--) {
        Subword* subword = subwords.elements[i];
        line = substr_replace(subword->word, subword->start, subword->end, oldLine = line);
        free(oldLine);
        free(subword->word);
    }

    ArrayList_free(&subwords, true);
    return line;
}

static void removeVariablesOutOfScope(ShaderCode* shaderCode) {
    for (int i = shaderCode->variables.size-1; i >= 0; i--) {
        ShaderVariable* variable = shaderCode->variables.entries[i].value;
        if (variable->outOfScope) {
            ArrayMap_removeAt(&shaderCode->variables, i);
            if (!variable->isMember) {
                MEMFREE(variable->name);
                free(variable);
            }
        }
    }
}

static void checkPreprocessorExtension(ShaderCode* shaderCode, char* line) {
    for (int i = 0; i < ARRAY_SIZE(allowedExtensions); i++) {
        if (strstr(line, allowedExtensions[i])) {
            char string[64];
            sprintf(string, "#define GD_%s 1", allowedExtensions[i] + 3);
            ArrayList_add(&shaderCode->lines, strdup(string));
            shaderCode->flags |= FLAG_SHADER_EXTENSIONS;
        }
    }
}

static char* replaceShaderExtensions(char* line) {
    for (int i = 0; i < ARRAY_SIZE(allowedExtensions); i++) {
        if (strstr(line, allowedExtensions[i])) {
            char replace[64];
            sprintf(replace, "GD_%s", allowedExtensions[i] + 3);
            char* oldLine = line;
            line = str_replace(allowedExtensions[i], replace, oldLine = line);
            free(oldLine);
        }
    }
    return line;
}

static void iterateShaderSource(ShaderObject* shader, char* code, int size) {
    FOREACH_LINE(code, size,
        char* newLine = strdup(ltrim(line));

        bool skipLine = false;
        if (startsWithPreprocessor("version ", newLine)) {
            char version[4];
            shader->code.version = strnums(newLine, version) > 0 ? strtol(version, NULL, 10) : 0;
            skipLine = true;
        }
        else if (startsWithPreprocessor("extension ", newLine)) {
            checkPreprocessorExtension(&shader->code, newLine);
            skipLine = true;
        }
        else skipLine = startsWithPreprocessor("pragma ", newLine) ||
                        startsWithPreprocessor("line ", newLine) ||
                        cstartswith("//", newLine);

        extractDefinedMacro(&shader->code, newLine);

        if (!skipLine) {
            extractShaderDataTypes(&shader->code, newLine);
            newLine = replaceReservedWords(shader, newLine);

            newLine = implicitConvertFunctionParams(&shader->code, newLine);
            if (shader->code.flags & FLAG_HAS_OPERATORS) newLine = implicitConvertIntToFloat(&shader->code, newLine);
            if (shader->code.flags & FLAG_SHADER_EXTENSIONS) newLine = replaceShaderExtensions(newLine);

            removeVariablesOutOfScope(&shader->code);
            ArrayList_add(&shader->code.lines, newLine);
        }
        else free(newLine);
    );
}

static void insertCodeLine(ShaderObject* shader, int index, char* line) {
    ArrayList_addAt(&shader->code.lines, index, line);

    for (int i = 0; i < shader->code.functions.size; i++) {
        ShaderFunction* function = shader->code.functions.elements[i];
        if (function->lineEnd >= index) {
            function->lineStart++;
            function->lineEnd++;
        }
    }

    for (int i = 0; i < shader->code.variables.size; i++) {
        ShaderVariable* variable = shader->code.variables.entries[i].value;
        if (variable->lineStart >= index) variable->lineStart++;
    }
}

static void removeCodeLine(ShaderObject* shader, int index) {
    void* line = ArrayList_removeAt(&shader->code.lines, index);
    MEMFREE(line);

    for (int i = 0; i < shader->code.functions.size; i++) {
        ShaderFunction* function = shader->code.functions.elements[i];
        if (function->lineStart >= index) {
            function->lineStart--;
            function->lineEnd--;
        }
    }

    for (int i = 0; i < shader->code.variables.size; i++) {
        ShaderVariable* variable = shader->code.variables.entries[i].value;
        if (variable->lineStart >= index) variable->lineStart--;
    }
}

static int countMainFunctions(ShaderObject* shader) {
    int count = 0;
    for (int i = 0; i < shader->code.functions.size; i++) {
        ShaderFunction* function = shader->code.functions.elements[i];
        if (strcmp(function->name, "main") == 0) count++;
    }
    return count;
}

static void checkGlobalInitializerConsts(ShaderObject* shader) {
    for (int i = 0; i < shader->code.variables.size; i++) {
        ShaderVariable* variable = shader->code.variables.entries[i].value;
        if (variable->scopeId > 0) continue;

        if (variable->typeQualifier == TYPE_QUALIFIER_NONE || (variable->typeQualifier == TYPE_QUALIFIER_UNIFORM &&
                                                               shader->code.version <= 120)) {
            char* oldLine = shader->code.lines.elements[variable->lineStart];
            MARK_START_COMMENT(oldLine);
            bool hasEquals = strchr(oldLine, '=') != NULL;
            UNMARK_START_COMMENT();

            if (hasEquals) {
                char* newLine;
                if (variable->typeQualifier == TYPE_QUALIFIER_NONE) {
                    newLine = strjoin(0, 2, "const ", oldLine);
                }
                else newLine = str_replace("uniform ", "const ", oldLine);
                shader->code.lines.elements[variable->lineStart] = newLine;
                free(oldLine);
                variable->typeQualifier = TYPE_QUALIFIER_CONST;
            }
        }
    }
}

static void removeReservedBuiltinNames(ShaderObject* shader) {
    const char* names[] = {"gl_Position", "gl_VertexID", "gl_InstanceID", "gl_FragCoord"};
    for (int i = 0; i < ARRAY_SIZE(names); i++) {
        ShaderVariable* variable = ArrayMap_get(&shader->code.variables, names[i]);
        if (variable) removeCodeLine(shader, variable->lineStart);
    }
}

static void injectBuiltinVariables(ShaderProgram* program, ShaderObject* shader) {
    int numMainFunctions = countMainFunctions(shader);
    ShaderFunction* mainFunctions[numMainFunctions];
    for (int i = 0, j = 0; i < shader->code.functions.size; i++) {
        ShaderFunction* shaderFunction = shader->code.functions.elements[i];
        if (strcmp(shaderFunction->name, "main") == 0) mainFunctions[j++] = shaderFunction;
    }

    int head = 0;
    insertCodeLine(shader, head++, strdup("#version 320 es"));
    if (shader->type == GL_FRAGMENT_SHADER) {
        insertCodeLine(shader, head++, strdup("precision highp float;"));
        insertCodeLine(shader, head++, strdup("precision highp int;"));
        insertCodeLine(shader, head++, strdup("precision highp sampler2DShadow;"));
    }

    const char* prefix = shader->type == GL_VERTEX_SHADER ? "out" : "in";

    if (shader->type == GL_VERTEX_SHADER) {
        if ((shader->code.flags & FLAG_BUILTIN_COLOR)) insertCodeLine(shader, head++, strdup("in vec4 gd_Color;"));
        if ((shader->code.flags & FLAG_BUILTIN_SECONDARY_COLOR)) insertCodeLine(shader, head++, strdup("in vec4 gd_SecondaryColor;"));
        if ((shader->code.flags & FLAG_BUILTIN_VERTEX)) insertCodeLine(shader, head++, strdup("in vec4 gd_Vertex;"));
        if ((shader->code.flags & FLAG_BUILTIN_NORMAL)) insertCodeLine(shader, head++, strdup("in vec3 gd_Normal;"));

        if (shader->code.flags & FLAG_BUILTIN_MULTITEXCOORD) {
            char text[64];
            for (int i = 0; i < MAX_TEXCOORDS; i++) {
                sprintf(text, "in vec4 gd_MultiTexCoord%d;", i);
                insertCodeLine(shader, head++, strdup(text));
            }
        }

        if (shader->code.flags & FLAG_BUILTIN_FOGCOORD) insertCodeLine(shader, head++, strdup("in float gd_FogCoord;"));
    }

    if ((shader->code.flags & FLAG_BUILTIN_FRONT_COLOR) || (shader->code.flags & FLAG_BUILTIN_BACK_COLOR)) {
        insertCodeLine(shader, head++, strjoin(' ', 2, prefix, "vec4 gd_FrontColor;"));
        insertCodeLine(shader, head++, strjoin(' ', 2, prefix, "vec4 gd_BackColor;"));

        if (shader->type == GL_VERTEX_SHADER && !(shader->code.flags & FLAG_BUILTIN_BACK_COLOR)) {
            for (int i = 0; i < numMainFunctions; i++) insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup("gd_BackColor = gd_FrontColor;"));
        }
    }

    if ((shader->code.flags & FLAG_BUILTIN_FRONT_SECONDARY_COLOR) || (shader->code.flags & FLAG_BUILTIN_BACK_SECONDARY_COLOR)) {
        insertCodeLine(shader, head++, strjoin(' ', 2, prefix, "vec4 gd_FrontSecondaryColor;"));
        insertCodeLine(shader, head++, strjoin(' ', 2, prefix, "vec4 gd_BackSecondaryColor;"));

        if (shader->type == GL_VERTEX_SHADER && !(shader->code.flags & FLAG_BUILTIN_BACK_SECONDARY_COLOR)) {
            for (int i = 0; i < numMainFunctions; i++) insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup("gd_BackSecondaryColor = gd_FrontSecondaryColor;"));
        }
    }

    if (shader->code.flags & FLAG_BUILTIN_TEXCOORD) {
        char text[64];
        sprintf(text, "%s vec4 gd_TexCoord[%d];", prefix, MAX_TEXCOORDS);
        insertCodeLine(shader, head++, strdup(text));
    }

    if (shader->code.flags & FLAG_BUILTIN_FOG_FRAGCOORD) {
        insertCodeLine(shader, head++, strjoin(' ', 2, prefix, "float gd_FogFragCoord;"));
    }

    if (shader->code.flags & FLAG_BUILTIN_FRAG_COLOR) insertCodeLine(shader, head++, strdup("out vec4 gd_FragColor;"));
    if (shader->code.flags & FLAG_BUILTIN_MODEL_VIEW_MATRIX) insertCodeLine(shader, head++, strdup("uniform mat4 gd_ModelViewMatrix;"));
    if (shader->code.flags & FLAG_BUILTIN_PROJECTION_MATRIX) insertCodeLine(shader, head++, strdup("uniform mat4 gd_ProjectionMatrix;"));
    if (shader->code.flags & FLAG_BUILTIN_MODEL_VIEW_PROJECTION_MATRIX) insertCodeLine(shader, head++, strdup("uniform mat4 gd_ModelViewProjectionMatrix;"));

    if (shader->code.flags & FLAG_BUILTIN_TEXTURE_MATRIX) {
        char text[64];
        sprintf(text, "uniform mat4 gd_TextureMatrix[%d];", MAX_TEXCOORDS);
        insertCodeLine(shader, head++, strdup(text));
    }

    if (shader->code.flags & FLAG_BUILTIN_FOG) {
        char* fogShader = strdup(SHADER_CHUNK_FOG);
        FOREACH_LINE(fogShader, strlen(fogShader)+1, insertCodeLine(shader, head++, strdup(line)););
        free(fogShader);
    }

    if (shader->code.version < 140 && shader->type == GL_FRAGMENT_SHADER) {
        insertCodeLine(shader, head++, strdup("#define GD_ALPHA_TEST 1"));

        char* alphaTestShader = strdup(SHADER_CHUNK_ALPHA_TEST);
        for (int i = 0; i < numMainFunctions; i++) {
            FOREACH_LINE(alphaTestShader, strlen(alphaTestShader)+1, insertCodeLine(shader, mainFunctions[i]->lineStart, strdup(line)););

            insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup("#if GD_ALPHA_TEST"));
            ShaderVariable* colorOut = NULL;
            if (program->fragDataLocations.size > 0) {
                char* fragColorName = SparseArray_get(&program->fragDataLocations, 0);
                if (fragColorName) colorOut = ArrayMap_get(&shader->code.variables, fragColorName);
            }
            else if ((shader->code.flags & FLAG_BUILTIN_FRAG_COLOR)) {
                insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup("applyAlphaTest(gd_FragColor.a);"));
            }
            else {
                for (int j = 0; j < shader->code.variables.size; j++) {
                    ShaderVariable* variable = shader->code.variables.entries[j].value;
                    if (variable->typeQualifier == TYPE_QUALIFIER_OUT) {
                        if (!colorOut || variable->location == 0) colorOut = variable;
                    }
                }
            }

            if (colorOut) {
                ShaderVariable* variable = ArrayMap_get(&shader->code.variables, colorOut->name);
                char string[128];
                if (variable && variable->arraySize > 0) {
                    sprintf(string, "applyAlphaTest(%s[0].a);", colorOut->name);
                }
                else sprintf(string, "applyAlphaTest(%s.a);", colorOut->name);
                insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup(string));
            }

            insertCodeLine(shader, mainFunctions[i]->lineEnd, strdup("#endif"));
        }
        free(alphaTestShader);
    }
}

static void destroyShaderObject(GLClientState* clientState, ShaderObject* shader) {
    if (!shader) return;
    for (int i = 0; i < clientState->programs->size; i++) {
        ShaderProgram* program = clientState->programs->entries[i].value;
        ArrayList_remove(&program->attachedShaders, shader);
    }

    for (int i = 0; i < shader->code.functions.size; i++) {
        ShaderFunction* function = shader->code.functions.elements[i];
        IntArray_clear(&function->paramTypes);
        MEMFREE(function->name);
    }

    for (int i = 0; i < shader->code.variables.size; i++) {
        ShaderVariable* variable = shader->code.variables.entries[i].value;
        ShaderVariable* next = variable->members;
        while (next) {
            ShaderVariable* member = next;
            next = next->members;
            MEMFREE(member->name);
            MEMFREE(member);
        }

        MEMFREE(variable->name);
        MEMFREE(variable->blockName);
        MEMFREE(variable);
    }

    ArrayList_free(&shader->code.functions, true);
    ArrayMap_free(&shader->code.variables, false, false);
    ArrayMap_free(&shader->code.definedMacros, true, true);
    ArrayList_free(&shader->code.lines, true);
    free(shader);
}

static void destroyShaderProgram(ShaderProgram* program) {
    if (!program) return;
    SparseArray_free(&program->fragDataLocations, true);
    free(program);
}

void ShaderConverter_setShaderSource(GLuint shaderId, GLsizei count, ArrayBuffer* inputBuffer) {
    GLX_CONTEXT_LOCK();
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);
    if (!shader) {
        GLX_CONTEXT_UNLOCK();
        return;
    }

    for (int i = 0; i < count; i++) {
        int size = ArrayBuffer_getInt(inputBuffer);
        char* code = ArrayBuffer_getBytes(inputBuffer, size);

        iterateShaderSource(shader, code, size);
    }

    removeReservedBuiltinNames(shader);
    checkGlobalInitializerConsts(shader);
    GLX_CONTEXT_UNLOCK();
}

void ShaderConverter_getShaderSource(ShaderObject* shader, ArrayBuffer* outSource) {
    for (int i = 0; i < shader->code.lines.size; i++) {
        char* line = shader->code.lines.elements[i];
        ArrayBuffer_putBytes(outSource, line, strlen(line));
        ArrayBuffer_put(outSource, '\n');
    }
    ArrayBuffer_put(outSource, '\0');
}

GLuint ShaderConverter_createShader(GLenum type) {
    GLX_CONTEXT_LOCK();
    GLuint shaderId = glCreateShader(type);
    ShaderObject* shaderObject = calloc(1, sizeof(ShaderObject));
    shaderObject->id = shaderId;
    shaderObject->type = type;

    SparseArray_put(currentRenderer->clientState.shaders, shaderId, shaderObject);
    GLX_CONTEXT_UNLOCK();
    return shaderId;
}

ShaderObject* ShaderConverter_getShader(GLuint shaderId) {
    GLX_CONTEXT_LOCK();
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);
    GLX_CONTEXT_UNLOCK();
    return shader;
}

void ShaderConverter_deleteShader(GLuint shaderId) {
    GLX_CONTEXT_LOCK();
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);
    if (shader) {
        shader->deleted = true;
        if (!shader->attached) {
            glDeleteShader(shader->id);
            SparseArray_remove(currentRenderer->clientState.shaders, shader->id);
            destroyShaderObject(&currentRenderer->clientState, shader);
        }
    }
    GLX_CONTEXT_UNLOCK();
}

ShaderProgram* ShaderConverter_getProgram(GLuint programId) {
    if (programId == 0) return NULL;
    GLX_CONTEXT_LOCK();
    ShaderProgram* program = SparseArray_get(currentRenderer->clientState.programs, programId);
    GLX_CONTEXT_UNLOCK();
    return program;
}

void ShaderConverter_deleteProgram(GLuint programId) {
    GLX_CONTEXT_LOCK();
    ShaderProgram* program = SparseArray_get(currentRenderer->clientState.programs, programId);
    if (program) {
        while (program->attachedShaders.size > 0) {
            ShaderObject* shader = ArrayList_removeAt(&program->attachedShaders, 0);
            if (shader->deleted) {
                glDeleteShader(shader->id);
                SparseArray_remove(currentRenderer->clientState.shaders, shader->id);
                destroyShaderObject(&currentRenderer->clientState, shader);
            }
        }

        SparseArray_remove(currentRenderer->clientState.programs, programId);
        destroyShaderProgram(program);
    }

    glDeleteProgram(programId);
    GLX_CONTEXT_UNLOCK();
}

GLuint ShaderConverter_createProgram() {
    GLX_CONTEXT_LOCK();
    GLuint programId = glCreateProgram();
    ShaderProgram* program = calloc(1, sizeof(ShaderProgram));
    program->id = programId;
    for (int i = 0; i < sizeof(program->location); i += sizeof(GLint)) *(GLint*)((char*)&program->location + i) = -1;

    SparseArray_put(currentRenderer->clientState.programs, programId, program);
    GLX_CONTEXT_UNLOCK();
    return programId;
}

void ShaderConverter_attachShader(GLuint programId, GLuint shaderId) {
    GLX_CONTEXT_LOCK();
    ShaderProgram* program = SparseArray_get(currentRenderer->clientState.programs, programId);
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);

    if (program && shader && ArrayList_indexOf(&program->attachedShaders, shader) == -1) {
        shader->attached = true;
        ArrayList_add(&program->attachedShaders, shader);
    }
    GLX_CONTEXT_UNLOCK();
}

void ShaderConverter_detachShader(GLuint programId, GLuint shaderId) {
    GLX_CONTEXT_LOCK();
    ShaderProgram* program = SparseArray_get(currentRenderer->clientState.programs, programId);
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);

    if (program && shader) {
        shader->attached = false;
        ArrayList_remove(&program->attachedShaders, shader);
        if (shader->deleted) {
            glDeleteShader(shader->id);
            SparseArray_remove(currentRenderer->clientState.shaders, shader->id);
            destroyShaderObject(&currentRenderer->clientState, shader);
        }
    }
    GLX_CONTEXT_UNLOCK();
}

static void compileShaderObject(ShaderProgram* program, ShaderObject* shader) {
    ArrayBuffer shaderSource = {0};
    ShaderConverter_getShaderSource(shader, &shaderSource);

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
    printShaderLines(shader->type, shader->id, program->id, shaderSource.buffer, strlen(shaderSource.buffer));
#endif

    glShaderSource(shader->id, 1, (const GLchar* const*)&shaderSource.buffer, NULL);
    glCompileShader(shader->id);
    ArrayBuffer_free(&shaderSource);

    GLint compileStatus;
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &compileStatus);
    shader->compileStatus = compileStatus ? COMPILE_STATUS_SUCCESS : COMPILE_STATUS_ERROR;

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
    if (!compileStatus) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader->id, 512, NULL, infoLog);
        println("gladio: %s:%d compile error \n%s\n", glEnumToString(shader->type), shader->id, infoLog);
        exit(1);
    }
#endif
}

static ShaderFunction* findOriginFunction(ShaderFunction* targetFunction, ArrayList* functions) {
    for (int i = 0; i < functions->size; i++) {
        ShaderFunction* originFunction = functions->elements[i];
        if (strcmp(originFunction->name, targetFunction->name) == 0 && originFunction->paramTypes.size == targetFunction->paramTypes.size) {
            bool found = true;
            for (int j = 0; j < originFunction->paramTypes.size && found; j++) {
                if (originFunction->paramTypes.values[j] != targetFunction->paramTypes.values[j]) found = false;
            }
            if (found) return originFunction;
        }
    }
    return NULL;
}

static void mergeSeparateShaders(ShaderProgram* program, ShaderObject* mainShader) {
    int head = 0;
    for (int i = 0, j, k; i < program->attachedShaders.size; i++) {
        ShaderObject* separateShader = program->attachedShaders.elements[i];
        if (separateShader != mainShader && separateShader->type == mainShader->type && countMainFunctions(separateShader) == 0) {
            for (j = 0; j < separateShader->code.variables.size; j++) {
                ShaderVariable* targetVariable = separateShader->code.variables.entries[j].value;
                ShaderVariable* originVariable = ArrayMap_get(&mainShader->code.variables, targetVariable->name);
                if (!originVariable && targetVariable->scopeId == 0 && targetVariable->groupId == 0) {
                    insertCodeLine(mainShader, head++, stringifyShaderVariable(targetVariable));
                }
            }

            for (j = 0; j < separateShader->code.functions.size; j++) {
                ShaderFunction* targetFunction = separateShader->code.functions.elements[j];
                ShaderFunction* originFunction = findOriginFunction(targetFunction, &mainShader->code.functions);
                if (originFunction && !originFunction->hasBody && targetFunction->hasBody) {
                    int lineIndex = originFunction->lineStart;
                    removeCodeLine(mainShader, lineIndex);
                    for (k = targetFunction->lineStart; k <= targetFunction->lineEnd; k++) {
                        char* line = separateShader->code.lines.elements[k];
                        insertCodeLine(mainShader, lineIndex++, strdup(line));
                    }
                }
            }

            mainShader->code.flags |= separateShader->code.flags;
        }
    }
}

static void linkShaderProgram(ShaderProgram* program) {
    glLinkProgram(program->id);

#if IS_DEBUG_ENABLED(DEBUG_MODE_SHADER_INFO)
    GLint linkStatus;
    glGetProgramiv(program->id, GL_LINK_STATUS, &linkStatus);

    if (!linkStatus) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program->id, 512, NULL, infoLog);
        char attachedShaders[64] = {0};
        for (int i = 0; i < program->attachedShaders.size; i++) {
            ShaderObject* shader = program->attachedShaders.elements[i];
            sprintf(attachedShaders, "%s%s%d", attachedShaders, i > 0 ? "," : "", shader->id);
        }
        println("gladio: program %d link error [shaders: %s] \n%s\n", program->id, attachedShaders, infoLog);
        exit(1);
    }
#endif

    bool hasBuiltinAttributes = false;
    bool hasBuiltinUniforms = false;
    for (int i = 0; i < program->attachedShaders.size; i++) {
        ShaderObject* shader = program->attachedShaders.elements[i];
        if ((shader->code.flags & (FLAG_BUILTIN_VERTEX | FLAG_BUILTIN_COLOR | FLAG_BUILTIN_NORMAL | FLAG_BUILTIN_MULTITEXCOORD))) {
            hasBuiltinAttributes = true;
            if (shader->code.flags & FLAG_BUILTIN_COLOR) program->hasBuiltinColor = true;
        }
        if ((shader->code.flags & (FLAG_BUILTIN_MODEL_VIEW_MATRIX |
                                   FLAG_BUILTIN_PROJECTION_MATRIX |
                                   FLAG_BUILTIN_MODEL_VIEW_PROJECTION_MATRIX |
                                   FLAG_BUILTIN_TEXTURE_MATRIX |
                                   FLAG_BUILTIN_FOG))) {
            hasBuiltinUniforms = true;
        }
    }

    if (hasBuiltinAttributes) {
        program->location.attributes[POSITION_ARRAY_INDEX] = glGetAttribLocation(program->id, "gd_Vertex");
        program->location.attributes[COLOR_ARRAY_INDEX] = glGetAttribLocation(program->id, "gd_Color");
        program->location.attributes[NORMAL_ARRAY_INDEX] = glGetAttribLocation(program->id, "gd_Normal");

        for (int i = 0, j = TEXCOORD_ARRAY_INDEX; i < MAX_TEXCOORDS; i++, j++) {
            char attribName[32];
            sprintf(attribName, "gd_MultiTexCoord%d", i);
            program->location.attributes[j] = glGetAttribLocation(program->id, attribName);
        }
    }

    program->location.alphaTest = glGetUniformLocation(program->id, "gd_AlphaTest");
    if (hasBuiltinUniforms) {
        program->location.modelViewMatrix = glGetUniformLocation(program->id, "gd_ModelViewMatrix");
        program->location.projectionMatrix = glGetUniformLocation(program->id, "gd_ProjectionMatrix");
        program->location.modelViewProjectionMatrix = glGetUniformLocation(program->id, "gd_ModelViewProjectionMatrix");

        char uniformName[32] = {0};
        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            sprintf(uniformName, "gd_TextureMatrix[%d]", i);
            program->location.textureMatrix[i] = glGetUniformLocation(program->id, uniformName);
        }
        
        ShaderMaterial_getFogUniformLocations(program->id, program->location.fog);
    }

    for (int i = 0; i < sizeof(program->location); i += sizeof(GLint)) {
        if (*(GLint*)((char*)&program->location + i) != -1) {
            program->hasBuiltinUniforms = true;
            break;
        }
    }
}

void ShaderConverter_linkProgram(GLuint programId) {
    GLX_CONTEXT_LOCK();
    ShaderProgram* program = SparseArray_get(currentRenderer->clientState.programs, programId);
    if (!program) {
        GLX_CONTEXT_UNLOCK();
        return;
    }

    for (int i = 0; i < program->attachedShaders.size; i++) {
        ShaderObject* shader = program->attachedShaders.elements[i];

        if (shader->compileStatus == COMPILE_STATUS_PENDING && countMainFunctions(shader) > 0) {
            mergeSeparateShaders(program, shader);
            injectBuiltinVariables(program, shader);
            compileShaderObject(program, shader);
        }

        if (shader->compileStatus == COMPILE_STATUS_SUCCESS) glAttachShader(programId, shader->id);
    }

    linkShaderProgram(program);
    GLX_CONTEXT_UNLOCK();
}

void ShaderConverter_getShaderiv(GLuint shaderId, GLenum pname, GLint* params) {
    GLX_CONTEXT_LOCK();
    ShaderObject* shader = SparseArray_get(currentRenderer->clientState.shaders, shaderId);
    *params = 0;
    if (!shader) {
        GLX_CONTEXT_UNLOCK();
        return;
    }

    switch (pname) {
        case GL_SHADER_TYPE:
            *params = shader->type;
            break;
        case GL_DELETE_STATUS:
            *params = shader->deleted;
            break;
        case GL_COMPILE_STATUS:
            *params = shader->compileStatus != COMPILE_STATUS_ERROR ? GL_TRUE : GL_FALSE;
            break;
        case GL_INFO_LOG_LENGTH:
            glGetShaderiv(shaderId, pname, params);
            break;
        default:
            println("gladio:getShaderiv: unimplemented pname %x", pname);
            break;
    }
    GLX_CONTEXT_UNLOCK();
}

void ShaderConverter_getProgramiv(GLuint target, GLenum pname, GLint* params) {
    *params = 0;
    switch (pname) {
        case GL_ACTIVE_ATTRIBUTES:
        case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
        case GL_ACTIVE_UNIFORM_BLOCKS:
        case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
        case GL_ACTIVE_UNIFORMS:
        case GL_ACTIVE_UNIFORM_MAX_LENGTH:
        case GL_ATTACHED_SHADERS:
        case GL_DELETE_STATUS:
        case GL_INFO_LOG_LENGTH:
        case GL_LINK_STATUS:
        case GL_PROGRAM_BINARY_RETRIEVABLE_HINT:
        case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
        case GL_TRANSFORM_FEEDBACK_VARYINGS:
        case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
        case GL_VALIDATE_STATUS:
            glGetProgramiv(target, pname, params);
            break;
        case GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB:
        case GL_MAX_PROGRAM_ENV_PARAMETERS_ARB:
            *params = target == GL_VERTEX_PROGRAM_ARB ? 96 : 24;
            break;
        case GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB:
        case GL_MAX_PROGRAM_INSTRUCTIONS_ARB:
            *params = 4096;
            break;
        case GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB:
        case GL_MAX_PROGRAM_TEMPORARIES_ARB:
        case GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB:
        case GL_MAX_PROGRAM_PARAMETERS_ARB:
            *params = 64;
            break;
        default:
            println("gladio:getProgramiv: unimplemented pname %x", pname);
            break;
    }
}

void ShaderConverter_updateBoundProgram() {
    if (!currentRenderer->clientState.program) return;
    ShaderProgram* program = currentRenderer->clientState.program;

    GLClientState* clientState = &currentRenderer->clientState;
    if (program->hasBuiltinColor && !clientState->vao->attribs[COLOR_ARRAY_INDEX].state && program->location.attributes[COLOR_ARRAY_INDEX] != -1) {
        GLRenderer_disableVertexAttribute(currentRenderer, program->location.attributes[COLOR_ARRAY_INDEX]);
        glVertexAttrib4fv(program->location.attributes[COLOR_ARRAY_INDEX], currentRenderer->state.color);
    }

    if (program->hasBuiltinUniforms) {
        if (program->location.alphaTest != -1) {
            glUniform2f(program->location.alphaTest, currentRenderer->state.alphaTest.enabled ? currentRenderer->state.alphaTest.func : GL_ALWAYS, currentRenderer->state.alphaTest.ref);
        }

        if (program->location.modelViewMatrix != -1) {
            glUniformMatrix4fv(program->location.modelViewMatrix, 1, GL_FALSE, GLRenderer_getMatrixFromStack(currentRenderer, MODEL_VIEW_MATRIX_INDEX));
        }

        if (program->location.projectionMatrix != -1) {
            glUniformMatrix4fv(program->location.projectionMatrix, 1, GL_FALSE, GLRenderer_getMatrixFromStack(currentRenderer, PROJECTION_MATRIX_INDEX));
        }

        if (program->location.modelViewProjectionMatrix != -1) {
            float matrix[16];
            mat4_multiply(matrix, GLRenderer_getMatrixFromStack(currentRenderer, MODEL_VIEW_MATRIX_INDEX), GLRenderer_getMatrixFromStack(currentRenderer, PROJECTION_MATRIX_INDEX));
            glUniformMatrix4fv(program->location.modelViewProjectionMatrix, 1, GL_FALSE, matrix);
        }

        for (int i = 0; i < MAX_TEXCOORDS; i++) {
            if (program->location.textureMatrix[i] != -1) {
                float* matrix = GLRenderer_getMatrixFromStack(currentRenderer, TEXTURE_MATRIX_INDEX);
                glUniformMatrix4fv(program->location.textureMatrix[i], 1, GL_FALSE, matrix);
            }
        }

        if (program->location.fog[0] != -1) {
            glUniform4fv(program->location.fog[0], 1, currentRenderer->state.fog.color);
            glUniform1f(program->location.fog[1], currentRenderer->state.fog.density);
            glUniform1f(program->location.fog[2], currentRenderer->state.fog.start);
            glUniform1f(program->location.fog[3], currentRenderer->state.fog.end);

            float scale = 1.0f / (currentRenderer->state.fog.end - currentRenderer->state.fog.start);
            glUniform1f(program->location.fog[4], scale);
        }
    }
}

void ShaderConverter_onDestroy(GLClientState* clientState) {
    for (int i = clientState->shaders->size-1; i >= 0; i--) {
        ShaderObject* shader = clientState->shaders->entries[i].value;
        destroyShaderObject(clientState, shader);
        SparseArray_removeAt(clientState->shaders, i);
    }

    for (int i = clientState->programs->size-1; i >= 0; i--) {
        ShaderProgram* program = clientState->programs->entries[i].value;
        destroyShaderProgram(program);
        SparseArray_removeAt(clientState->programs, i);
    }
}