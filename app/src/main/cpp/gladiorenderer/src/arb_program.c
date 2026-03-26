#include "arb_program.h"
#include "gl_context.h"

static GLuint maxProgramId = 1;

enum Inst {INST_NONE, INST_ABS, INST_ADD, INST_ARL, INST_CMP, INST_COS, INST_DP3, INST_DP4, INST_DPH, INST_DST, INST_EX2, INST_EXP, INST_FLR, INST_FRC, INST_KIL, INST_LG2, INST_LIT, INST_LOG, INST_LRP, INST_MAD, INST_MAX, INST_MIN, INST_MOV, INST_MUL, INST_POW, INST_RCP, INST_RSQ, INST_SCS, INST_SGE, INST_SIN, INST_SLT, INST_SUB, INST_SWZ, INST_TEX, INST_TXB, INST_TXP, INST_XPD};

struct ReservedGLName {
    char* name;
    char* replace;
    bool suffixUnit;
};

typedef struct ASMSource {
    char* string;
    int length;
    ArrayMap variables;
} ASMSource;

#define TYPE_QUALIFIER_ADDRESS 1
#define TYPE_QUALIFIER_ATTRIB 2
#define TYPE_QUALIFIER_PARAM 3
#define TYPE_QUALIFIER_ALIAS 4
#define TYPE_QUALIFIER_OUTPUT 5

static struct ReservedGLName reservedGLNames[] = {
    {"program.env", "gd_ProgramEnv", false},
    {"program.local", "gd_ProgramLocal", false},
    {"vertex.position", "gd_Vertex", false},
    {"vertex.normal", "gd_Normal", false},
    {"vertex.color", "gd_Color", false},
    {"vertex.texcoord", "gd_MultiTexCoord", true},
    {"fragment.texcoord", "gd_TexCoord", true},
    {"texture", "gd_Texture", true},
    {"result.position", "gl_Position", false},
    {"result.color", "gd_FragColor", false},
    {"result.texcoord", "gd_TexCoord", true}
};

static void getInstructionMap(ArrayMap* instructionMap) {
    static const char* instructions[] = {"ABS", "ADD", "ARL", "CMP", "COS", "DP3", "DP4", "DPH", "DST", "EX2", "EXP", "FLR", "FRC", "KIL", "LG2", "LIT", "LOG", "LRP", "MAD", "MAX", "MIN", "MOV", "MUL", "POW", "RCP", "RSQ", "SCS", "SGE", "SIN", "SLT", "SUB", "SWZ", "TEX", "TXB", "TXP", "XPD"};
    for (int i = 0; i < ARRAY_SIZE(instructions); i++) {
        ArrayMap_put(instructionMap, instructions[i], (void*)(uint64_t)(i + 1));
    }
}

static void extractInstOperands(char* line, ArrayList* operands) {
    char* chr = line;
    int groupCount = 0;
    int nameStart = -1;

    while (*chr) {
        if (*chr == '{') {
            groupCount++;
            if (groupCount > 0 && nameStart == -1) nameStart = chr - line;
        }
        else if (*chr == '}') {
            groupCount--;
        }
        else if (groupCount == 0 && nameStart == -1 && !isspace(*chr)) {
            nameStart = chr - line;
        }

        if (((*chr == ',' || *chr == ';') && groupCount == 0) && nameStart != -1) {
            int nameEnd = chr - line;
            while (isspace(line[nameEnd-1])) nameEnd--;
            ArrayList_add(operands, substr(line, nameStart, nameEnd - nameStart));
            nameStart = -1;
        }

        if (*chr == ';') break;
        chr++;
    }
}

static void extractArrayVariableParams(char* line, ArrayList* params) {
    char* chr = line;
    int groupCount = 0;
    int nameStart = -1;

    while (*chr) {
        if (*chr == '{') {
            groupCount++;
            if (groupCount > 1 && nameStart == -1) nameStart = chr - line;
        }
        else if (*chr == '}') {
            groupCount--;
        }
        else if (groupCount == 1 && nameStart == -1 && !isspace(*chr)) {
            nameStart = chr - line;
        }

        if (((*chr == ',' && groupCount == 1) || (*chr == '}' && groupCount == 0)) && nameStart != -1) {
            int nameEnd = chr - line;
            while (isspace(line[nameEnd-1])) nameEnd--;
            ArrayList_add(params, substr(line, nameStart, nameEnd - nameStart));
            nameStart = -1;
        }

        if (*chr == '}' && groupCount == 0) break;
        chr++;
    }
}

static bool extractVariableArrayRangeIndex(char* operand, int* range) {
    char string[64];
    substrv(operand, '[', ']', string);
    char* value = trim(string);
    char* dots = strstr(string, "..");
    range[0] = -1;
    range[1] = -1;
    if (dots) {
        dots[0] = '\0';
        if (is_int(value) && is_int(dots+2)) {
            range[0] = strtol(value, NULL, 10);
            range[1] = strtol(dots+2, NULL, 10);
        }
        dots[0] = '.';
    }
    return range[0] >= 0 && range[1] >= 0;
}

static void parseInstOperand(char* operand, ASMSource* asmSource, ArrayBuffer* shaderCode) {
    if (operand[0] == '{') {
        ArrayList params = {0};
        extractArrayVariableParams(operand, &params);

        ArrayBuffer_putString(shaderCode, "vec4(");
        for (int i = 0; i < params.size; i++) {
            char* elementValue = params.elements[i];
            if (i > 0) ArrayBuffer_putString(shaderCode, ", ");
            parseInstOperand(elementValue, asmSource, shaderCode);
        }
        ArrayBuffer_putString(shaderCode, ")");
        ArrayList_free(&params, true);
    }
    else {
        char* semicolon = strchr(operand, ';');
        int nameEnd = -1;
        char oldChar;
        if (semicolon) {
            nameEnd = semicolon - operand;
            while (isspace(operand[nameEnd-1])) nameEnd--;
            oldChar = operand[nameEnd];
            operand[nameEnd] = '\0';
        }

        if (is_int(operand)) {
            ArrayBuffer_putString(shaderCode, "%s.0", operand);
        }
        else if (is_float(operand)) {
            ArrayBuffer_putString(shaderCode, operand);
        }
        else {
            MARK_VARIABLE_NAME(operand);
            void* variableType = ArrayMap_get(&asmSource->variables, operand);
            UNMARK_VARIABLE_NAME(operand);

            if (variableType) {
                ArrayBuffer_putString(shaderCode, operand);
            }
            else {
                for (int i = 0; i < ARRAY_SIZE(reservedGLNames); i++) {
                    if (cstartswith(reservedGLNames[i].name, operand)) {
                        if (cstartswith("program.", operand)) {
                            int range[2];
                            if (extractVariableArrayRangeIndex(operand, range)) {
                                for (int j = range[0]; j <= range[1]; j++) {
                                    if (j > range[0]) ArrayBuffer_putString(shaderCode, ", ");
                                    ArrayBuffer_putString(shaderCode, "%s[%d]", reservedGLNames[i].replace, j);
                                }
                                continue;
                            }
                        }

                        if (reservedGLNames[i].suffixUnit) {
                            int index = extractVariableArrayIndex(operand);
                            ArrayBuffer_putString(shaderCode, "%s%d", reservedGLNames[i].replace, MAX(index, 0));
                        }
                        else ArrayBuffer_putString(shaderCode, "%s%s", reservedGLNames[i].replace, operand + strlen(reservedGLNames[i].name));
                    }
                }
            }
        }

        if (nameEnd != -1) operand[nameEnd] = oldChar;
    }
}

static void parseDataTypeQualifier(int type, char* line, ASMSource* asmSource, ArrayBuffer* shaderCode) {
    char* wordEnd = NULL;
    char* wordStart = strwrd(line, NULL, &wordEnd);

    if (wordStart) {
        char oldChar = *wordEnd;
        *wordEnd = '\0';
        ArrayBuffer_putString(shaderCode, "vec4 %s", wordStart);
        ArrayMap_put(&asmSource->variables, strdup(wordStart), (void*)(uint64_t)type);
        *wordEnd = oldChar;
    }
    else return;

    int arraySize = wordEnd && wordEnd[0] == '[' ? extractVariableArrayIndex(line) : 0;
    char* operand = strchr(line, '=');
    if (operand && ++operand) {
        operand = ltrim(operand);
        if (arraySize > 0 && operand[0] == '{') {
            ArrayList params = {0};
            extractArrayVariableParams(operand, &params);
            if (arraySize == INT32_MAX) arraySize = params.size;

            ArrayBuffer_putString(shaderCode, "[%d] = vec4[%d](", arraySize, arraySize);
            for (int i = 0; i < params.size; i++) {
                char* elementValue = params.elements[i];
                if (i > 0) ArrayBuffer_putString(shaderCode, ", ");
                parseInstOperand(elementValue, asmSource, shaderCode);
            }
            ArrayBuffer_putString(shaderCode, ")");
            ArrayList_free(&params, true);
        }
        else {
            ArrayBuffer_putString(shaderCode, " = vec4(");
            parseInstOperand(operand, asmSource, shaderCode);
            ArrayBuffer_putString(shaderCode, ")");
        }
    }

    ArrayBuffer_putString(shaderCode, ";\n");
}

static char* getOperandVectorType(char* operand) {
    for (int i = 0; i < ARRAY_SIZE(reservedGLNames); i++) {
        if (cstartswith(reservedGLNames[i].name, operand)) return NULL;
    }

    char* dot = strrchr(operand, '.');
    if (!dot) return NULL;
    char* component = dot + 1;

    int componentCount = 0;
    int i = 0;
    while (*component && i++ < 4) {
        switch (*component) {
            case 'x':
            case 'y':
            case 'z':
            case 'w':
            case 'r':
            case 'g':
            case 'b':
            case 'a':
            case 's':
            case 't':
            case 'q':
                componentCount++;
                break;
            default:
                return NULL;
        }
        component++;
    }

    switch (componentCount) {
        case 1:
            return "float";
        case 2:
            return "vec2";
        case 3:
            return "vec3";
        case 4:
            return "vec4";
        default:
            return NULL;
    }
}

static void iterateASMLine(char* line, ArrayMap* instructionMap, ASMSource* asmSource, ArrayBuffer* shaderCode) {
    line = ltrim(line);
    char* chr = line;
    int wordStart = -1;

    while (*chr) {
        if (*chr == ';') {
            iterateASMLine(line + 1, instructionMap, asmSource, shaderCode);
            break;
        }

        if (isupper(*chr) || (wordStart != -1 && isdigit(*chr))) {
            if (wordStart == -1) wordStart = chr - line;
        }
        else if (wordStart != -1) {
            char* word = line + wordStart;

            char oldChar = *chr;
            *chr = '\0';

            if (strcmp(word, "ADDRESS") == 0 ||
                strcmp(word, "TEMP") == 0) {
                parseDataTypeQualifier(TYPE_QUALIFIER_ADDRESS, chr + 1, asmSource, shaderCode);
            }
            else if (strcmp(word, "ATTRIB") == 0) {
                parseDataTypeQualifier(TYPE_QUALIFIER_ATTRIB, chr + 1, asmSource, shaderCode);
            }
            else if (strcmp(word, "PARAM") == 0) {
                parseDataTypeQualifier(TYPE_QUALIFIER_PARAM, chr + 1, asmSource, shaderCode);
            }
            else if (strcmp(word, "OUTPUT") == 0) {
                parseDataTypeQualifier(TYPE_QUALIFIER_OUTPUT, chr + 1, asmSource, shaderCode);
            }
            else if (strcmp(word, "ALIAS") == 0) {
                println("gladio: unimplemented asm type qualifier ALIAS");
            }
            else {
                void* value = ArrayMap_get(instructionMap, word);
                uint64_t inst = (uint64_t)value;
                *chr = oldChar;

                switch (inst) {
                    case INST_ADD:
                    case INST_SUB:
                    case INST_MUL:
                    case INST_ARL:
                    case INST_MOV: {
                        ArrayList operands = {0};
                        extractInstOperands(chr + 1, &operands);

                        char operator = '\0';
                        switch (inst) {
                            case INST_ADD:
                                operator = '+';
                                break;
                            case INST_SUB:
                                operator = '-';
                                break;
                            case INST_MUL:
                                operator = '*';
                                break;
                        }

                        char* outputType = NULL;
                        for (int i = 0; i < operands.size; i++) {
                            char* operand = operands.elements[i];
                            if (i == 0) {
                                outputType = getOperandVectorType(operand);
                            }
                            else if (i == 1) {
                                if (outputType) {
                                    ArrayBuffer_putString(shaderCode, " = %s(", outputType);
                                }
                                else ArrayBuffer_putString(shaderCode, " = ");
                            }
                            else if (i > 1) {
                                ArrayBuffer_putString(shaderCode, " %c ", operator);
                            }
                            parseInstOperand(operand, asmSource, shaderCode);
                        }

                        ArrayList_free(&operands, true);
                        ArrayBuffer_putString(shaderCode, outputType ? ");\n" : ";\n");
                        break;
                    }
                    case INST_ABS:
                    case INST_FLR:
                    case INST_FRC: {
                        ArrayList operands = {0};
                        extractInstOperands(chr + 1, &operands);

                        char* funcName = NULL;
                        switch (inst) {
                            case INST_ABS:
                                funcName = "abs";
                                break;
                            case INST_FLR:
                                funcName = "floor";
                                break;
                            case INST_FRC:
                                funcName = "frac";
                                break;
                        }

                        for (int i = 0; i < operands.size; i++) {
                            char* operand = operands.elements[i];
                            if (i == 1) ArrayBuffer_putString(shaderCode, " = %s(", funcName);
                            parseInstOperand(operand, asmSource, shaderCode);
                        }

                        ArrayList_free(&operands, true);
                        ArrayBuffer_putString(shaderCode, ");\n");
                        break;
                    }
                    case INST_TEX: {
                        ArrayList operands = {0};
                        extractInstOperands(chr + 1, &operands);

                        if (operands.size == 4) {
                            parseInstOperand(operands.elements[0], asmSource, shaderCode);
                            ArrayBuffer_putString(shaderCode, " = texture(");
                            parseInstOperand(operands.elements[2], asmSource, shaderCode);
                            ArrayBuffer_putString(shaderCode, ", ");
                            parseInstOperand(operands.elements[1], asmSource, shaderCode);
                            ArrayBuffer_putString(shaderCode, ".xy);\n");
                        }

                        ArrayList_free(&operands, true);
                        break;
                    }
                    default:
                        *chr = '\0';
                        println("gladio: unimplemented asm instruction %s %p", word, value);
                        break;
                }
            }

            *chr = oldChar;
            wordStart = -1;

            while (*chr && *chr != ';') chr++;
        }

        if (*chr == '\0') break;
        chr++;
    }
}

static void convertASMSource(GLenum type, ASMSource* asmSource, ArrayBuffer* shaderCode) {
    bool codeStarted = false;
    ArrayMap instructionMap = {0};
    getInstructionMap(&instructionMap);

    int maxProgramLocalParams = 0;
    int maxProgramEnvParams = 0;
    ShaderConverter_getProgramiv(type, GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, &maxProgramLocalParams);
    ShaderConverter_getProgramiv(type, GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, &maxProgramEnvParams);

    ArrayBuffer_putString(shaderCode, "uniform vec4 gd_ProgramEnv[%d];\n", maxProgramEnvParams);
    ArrayBuffer_putString(shaderCode, "uniform vec4 gd_ProgramLocal[%d];\n", maxProgramLocalParams);
    ArrayBuffer_putString(shaderCode, "void main() {\n");

    FOREACH_LINE(asmSource->string, asmSource->length,
        char* newLine = ltrim(line);
        if (cstartswith("!!ARBvp1.0", newLine)) {
            codeStarted = true;
        }
        else if (cstartswith("!!ARBfp1.0", newLine)) {
            codeStarted = true;
        }
        else if (codeStarted && !cstartswith("#", newLine)) {
            iterateASMLine(newLine, &instructionMap, asmSource, shaderCode);
        }
        else if (cstartswith("END", newLine)) {
            codeStarted = false;
        }
    );

    ArrayBuffer_putString(shaderCode, "}\n");
    ArrayBuffer_put(shaderCode, '\0');
}

ARBProgram* ARBProgram_create() {
    GLX_CONTEXT_LOCK();
    ARBProgram* program = calloc(1, sizeof(ARBProgram));
    program->id = maxProgramId++;
    program->threadId = currentThreadId();
    SparseArray_put(currentRenderer->clientState.arbPrograms, program->id, program);
    GLX_CONTEXT_UNLOCK();
    return program;
}

ARBProgram* ARBProgram_get(GLuint programId) {
    if (programId == 0) return NULL;
    GLX_CONTEXT_LOCK();
    ARBProgram* program = SparseArray_get(currentRenderer->clientState.arbPrograms, programId);
    GLX_CONTEXT_UNLOCK();
    return program;
}

ARBProgram* ARBProgram_getBound(GLenum target) {
    return currentRenderer->clientState.arbProgram[indexOfGLTarget(target)];
}

void ARBProgram_bind(GLenum target, GLuint programId) {
    GLX_CONTEXT_LOCK();
    ARBProgram* program = SparseArray_get(currentRenderer->clientState.arbPrograms, programId);
    GLX_CONTEXT_UNLOCK();

    if (program) {
        if (target == GL_VERTEX_PROGRAM_ARB) {
            if (program->shaderId == 0) program->shaderId = ShaderConverter_createShader(GL_VERTEX_SHADER);
            program->type = GL_VERTEX_PROGRAM_ARB;
        }
        else if (target == GL_FRAGMENT_PROGRAM_ARB) {
            if (program->shaderId == 0) program->shaderId = ShaderConverter_createShader(GL_FRAGMENT_SHADER);
            program->type = GL_FRAGMENT_PROGRAM_ARB;
        }
    }

    currentRenderer->clientState.arbProgram[indexOfGLTarget(target)] = program;
}

void ARBProgram_setSource(ARBProgram* program, GLenum format, char* string, GLuint length) {
    if (!program || format != GL_PROGRAM_FORMAT_ASCII_ARB) return;

    ASMSource asmSource = {0};
    asmSource.string = string;
    asmSource.length = length;

    ArrayBuffer shaderCode = {0};
    convertASMSource(program->type, &asmSource, &shaderCode);
    program->shaderCode = shaderCode.buffer;
    program->asmSource = strdup(string);

    ArrayMap_free(&asmSource.variables, true, false);
}

void ARBProgram_delete(GLuint programId) {
    GLX_CONTEXT_LOCK();
    ARBProgram* program = SparseArray_get(currentRenderer->clientState.arbPrograms, programId);
    GLX_CONTEXT_UNLOCK();

    if (program) {
        if (program->shaderId > 0) ShaderConverter_deleteShader(program->shaderId);
        SparseArray_free(&program->envParams, true);
        SparseArray_free(&program->localParams, true);
        MEMFREE(program->shaderCode);
        MEMFREE(program->asmSource);
        free(program);

        GLX_CONTEXT_LOCK();
        SparseArray_remove(currentRenderer->clientState.arbPrograms, programId);
        GLX_CONTEXT_UNLOCK();
    }
}

void ARBProgram_setEnvParameter(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    GLX_CONTEXT_LOCK();
    int threadId = currentThreadId();
    SparseArray* arbPrograms = currentRenderer->clientState.arbPrograms;
    for (int i = 0; i < arbPrograms->size; i++) {
        ARBProgram* program = arbPrograms->entries[i].value;
        if (program->threadId == threadId && program->type == target) {
            float* vector = SparseArray_get(&program->envParams, index);
            if (!vector) {
                vector = malloc(4 * sizeof(float));
                SparseArray_put(&program->envParams, index, vector);
            }

            vector[0] = x;
            vector[1] = y;
            vector[2] = z;
            vector[3] = w;
        }
    }
    GLX_CONTEXT_UNLOCK();
}

void ARBProgram_setLocalParameter(ARBProgram* program, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (!program) return;
    float* vector = SparseArray_get(&program->localParams, index);
    if (!vector) {
        vector = malloc(4 * sizeof(float));
        SparseArray_put(&program->localParams, index, vector);
    }

    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
    vector[3] = w;
}

void ARBProgram_onDestroy(GLClientState* clientState) {
    SparseArray* arbPrograms = clientState->arbPrograms;
    for (int i = arbPrograms->size-1; i >= 0; i--) {
        ARBProgram* program = arbPrograms->entries[i].value;
        SparseArray_free(&program->envParams, true);
        SparseArray_free(&program->localParams, true);
        MEMFREE(program->shaderCode);
        MEMFREE(program->asmSource);
        free(program);
        SparseArray_removeAt(arbPrograms, i);
    }
}