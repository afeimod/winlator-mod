#include "shader_inspector.h"
#include "vulkan_helper.h"
#include "spirv.h"
#include "string_utils.h"

#define PACK_SPV_INST(opcode, instLength) ((opcode << 0) | (instLength << SpvWordCountShift))
#define ADD_SPV_INST(intArray, instLength, opcode, ...) \
    IntArray_addAll(intArray, instLength, PACK_SPV_INST(opcode, instLength), __VA_ARGS__)
#define IS_SPV_OP_TYPE(op) (op >= SpvOpTypeVoid && op <= SpvOpTypeForwardPointer)
#define CODE_START 5
#define MAX_ID_BOUND_OFFSET 3
#define SPV_INST_OPCODE(val) (val & SpvOpCodeMask)
#define SPV_INST_LENGTH(val) (val >> SpvWordCountShift)

#define WRITE_FILE(filename, code, sizeInBytes) \
    do { \
        char path[PATH_MAX] = {0}; \
        sprintf(path, APP_CACHE_DIR "/%s", filename); \
        FILE* f = fopen(path, "wb"); \
        fwrite((const char*)code, 1, sizeInBytes, f); \
        fclose(f); \
    } \
    while(0)

typedef struct VertexAnnotation {
    int location;
    int vecId;
    int varId;
    int varOffset;
    int signedness;
    VkFormat format;
} VertexAnnotation;

typedef struct MergeCode {
    int position;
    IntArray code;
} MergeCode;

typedef struct SpvVariableOffsets {
    int decorate;
    int variable;
    int pointer;
    int vector;
} SpvVariableOffsets;

typedef struct SpvInstFilter {
    uint32_t fields[4];
    uint32_t values[4];
    int count;
    int endIndex;
} SpvInstFilter;

typedef struct MergeSpvInstOptions {
    uint32_t* code;
    uint32_t codeSize;
    ArrayList* mergeCodes;
    int mergePosition;
    int fetchStart;
    SpvInstFilter fetchFilter;
} MergeSpvInstOptions;

static VkFormat getFallbackFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_USCALED:
            return VK_FORMAT_R8_UINT;
        case VK_FORMAT_R8_SSCALED:
            return VK_FORMAT_R8_SINT;
        case VK_FORMAT_R16_USCALED:
            return VK_FORMAT_R16_UINT;
        case VK_FORMAT_R16_SSCALED:
            return VK_FORMAT_R16_SINT;
        case VK_FORMAT_R8G8_USCALED:
            return VK_FORMAT_R8G8_UINT;
        case VK_FORMAT_R8G8_SSCALED:
            return VK_FORMAT_R8G8_SINT;
        case VK_FORMAT_R16G16_USCALED:
            return VK_FORMAT_R16G16_UINT;
        case VK_FORMAT_R16G16_SSCALED:
            return VK_FORMAT_R16G16_SINT;
        case VK_FORMAT_R16G16B16A16_USCALED:
            return VK_FORMAT_R16G16B16A16_UINT;
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VK_FORMAT_R8G8B8A8_USCALED:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return VK_FORMAT_R8G8B8A8_SINT;
        default:
            return format;
    }
}

static uint32_t fetchSpvInstIndex(uint32_t* code, uint32_t sizeInBytes, int start, SpvOp targetOpcode, int field, uint32_t value, bool stopOnOpFunction) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;
    SpvOp stopOpcode = stopOnOpFunction ? SpvOpFunction : SpvOpMax;

    for (int i = start, size = sizeInBytes / sizeof(int); i < size && opcode != stopOpcode;) {
        opcode = SPV_INST_OPCODE(code[i]);
        instLength = SPV_INST_LENGTH(code[i]);

        if (opcode == targetOpcode) {
            if (field != -1) {
                if (field < instLength && code[i+field] == value) return i;
            }
            else return i;
        }
        i += instLength;
    }

    return -1;
}

static uint32_t fetchSpvInstIndex2(uint32_t* code, uint32_t sizeInBytes, int start, SpvOp targetOpcode, SpvInstFilter* filter, bool stopOnOpFunction) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;
    SpvOp stopOpcode = stopOnOpFunction ? SpvOpFunction : SpvOpMax;
    int size = sizeInBytes / sizeof(int);
    int end = filter->endIndex > 0 && filter->endIndex < size ? filter->endIndex : size;

    for (int i = start, j; i < end && opcode != stopOpcode;) {
        opcode = SPV_INST_OPCODE(code[i]);
        instLength = SPV_INST_LENGTH(code[i]);

        if (opcode == targetOpcode && filter->count < instLength) {
            bool match = true;
            for (j = 0; j < filter->count && match; j++) match = code[i+filter->fields[j]] == filter->values[j];
            if (match) return i;
        }

        i += instLength;
    }

    return -1;
}

static uint32_t fetchSpvOpTypeIndex(uint32_t* code, uint32_t sizeInBytes, int start, int field, uint32_t value) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;

    for (int i = start, size = sizeInBytes / sizeof(int); i < size && opcode != SpvOpFunction;) {
        opcode = SPV_INST_OPCODE(code[i]);
        instLength = SPV_INST_LENGTH(code[i]);

        if (IS_SPV_OP_TYPE(opcode)) {
            if (field != -1) {
                if (field < instLength && code[i+field] == value) return i;
            }
            else return i;
        }
        i += instLength;
    }

    return -1;
}

static void fetchSpvVariableOffsets(ShaderModule* module, SpvStorageClass storageClass, ArrayList* result) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;

    for (int i = CODE_START, j, k, l, size = module->codeSize / sizeof(uint32_t); i < size && opcode != SpvOpFunction && !IS_SPV_OP_TYPE(opcode);) {
        opcode = SPV_INST_OPCODE(module->code[i]);
        instLength = SPV_INST_LENGTH(module->code[i]);

        if (opcode == SpvOpDecorate && module->code[i+2] == SpvDecorationLocation) {
            j = fetchSpvInstIndex(module->code, module->codeSize, i, SpvOpVariable, 2, module->code[i+1], true);
            if (j != -1 && module->code[j+3] == storageClass) {
                k = fetchSpvInstIndex(module->code, module->codeSize, i, SpvOpTypePointer, 1, module->code[j+1], true);
                if (k != -1 && module->code[k+2] == storageClass) {
                    l = fetchSpvInstIndex(module->code, module->codeSize, i, SpvOpTypeVector, 1, module->code[k+3], true);
                    if (l == -1) l = fetchSpvInstIndex(module->code, module->codeSize, i, SpvOpTypeFloat, 1, module->code[k+3], true);
                    if (l != -1) {
                        SpvVariableOffsets* variableOffsets = calloc(1, sizeof(SpvVariableOffsets));
                        variableOffsets->decorate = i;
                        variableOffsets->variable = j;
                        variableOffsets->pointer = k;
                        variableOffsets->vector = l;
                        ArrayList_add(result, variableOffsets);
                    }
                }
            }
        }

        i += instLength;
    }
}

static uint32_t mergeSpvInst(MergeSpvInstOptions* options, uint32_t instLength, ...) {
    va_list valist;
    va_start(valist, instLength);
    SpvOp opcode = va_arg(valist, SpvOp);

    uint32_t resultId = 0;
    int mergePosition = 0;
    int index = fetchSpvInstIndex2(options->code, options->codeSize, options->fetchStart, opcode, &options->fetchFilter, true);
    if (index == -1) {
        for (int i = 0; i < options->mergeCodes->size; i++) {
            MergeCode* mergeCode = options->mergeCodes->elements[i];
            int oldEndIndex = options->fetchFilter.endIndex;
            options->fetchFilter.endIndex = 0;
            index = fetchSpvInstIndex2(mergeCode->code.values, mergeCode->code.size * sizeof(int), 0, opcode, &options->fetchFilter, false);
            options->fetchFilter.endIndex = oldEndIndex;
            if (index != -1) {
                resultId = mergeCode->code.values[index+1];
                mergePosition = (i + 1) * -1;
                break;
            }
        }
    }
    else {
        resultId = options->code[index+1];
        mergePosition = index + SPV_INST_LENGTH(options->code[index]);
    }

    if (resultId > 0) {
        options->mergePosition = mergePosition;
        va_end(valist);
        return resultId;
    }

    int mergeCodeIdx = options->mergePosition < 0 ? (options->mergePosition * -1) - 1 : -1;
    MergeCode* mergeCode = mergeCodeIdx == -1 ? calloc(1, sizeof(MergeCode)) : options->mergeCodes->elements[mergeCodeIdx];

    ENSURE_ARRAY_CAPACITY(mergeCode->code.size + instLength, mergeCode->code.capacity, mergeCode->code.values, sizeof(uint32_t));
    mergeCode->code.values[mergeCode->code.size++] = PACK_SPV_INST(opcode, instLength);

    for (int i = 1; i < instLength; i++) {
        uint32_t value = va_arg(valist, uint32_t);
        if (value == UINT32_MAX) value = resultId = options->code[MAX_ID_BOUND_OFFSET]++;
        mergeCode->code.values[mergeCode->code.size++] = value;
    }
    va_end(valist);

    if (mergeCodeIdx == -1) {
        mergeCode->position = options->mergePosition;
        mergeCodeIdx = options->mergeCodes->size;
        ArrayList_add(options->mergeCodes, mergeCode);
    }

    options->mergePosition = (mergeCodeIdx + 1) * -1;
    return resultId;
}

static void convertInputVariableFormatScaled(ShaderModule* module, ArrayList* mergeCodes, int decorOffset, VertexAnnotation* vertexAnnotation) {
    int index = fetchSpvInstIndex(module->code, module->codeSize, decorOffset, SpvOpTypePointer, 1, module->code[vertexAnnotation->varOffset+1], true);
    int componentCount = 4;
    if (index != -1) {
        uint32_t typeVectorId = module->code[index+3];
        index = fetchSpvInstIndex(module->code, module->codeSize, decorOffset, SpvOpTypeVector, 1, typeVectorId, true);
        if (index != -1) componentCount = module->code[index+3];
    }

    int signedness;
    switch (vertexAnnotation->format) {
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R16G16B16A16_UINT:
            signedness = 0;
            break;
        default:
            signedness = 1;
            break;
    }

    index = fetchSpvOpTypeIndex(module->code, module->codeSize, decorOffset, -1, 0);
    if (index == -1) return;

    MergeSpvInstOptions typeIntOptions = {module->code, module->codeSize, mergeCodes, index, index, {{3}, {signedness}, 1, vertexAnnotation->varOffset}};
    uint32_t typeIntId = mergeSpvInst(&typeIntOptions, 4, SpvOpTypeInt, UINT32_MAX, 32, signedness);

    MergeSpvInstOptions typeVectorOptions = {module->code, module->codeSize, mergeCodes, typeIntOptions.mergePosition, index, {{2, 3}, {typeIntId, componentCount}, 2, vertexAnnotation->varOffset}};
    uint32_t typeVectorId = mergeSpvInst(&typeVectorOptions, 4, SpvOpTypeVector, UINT32_MAX, typeIntId, componentCount);

    MergeSpvInstOptions typePointerOptions = {module->code, module->codeSize, mergeCodes, typeVectorOptions.mergePosition, index, {{2, 3}, {SpvStorageClassInput, typeVectorId}, 2, vertexAnnotation->varOffset}};
    uint32_t typePointerId = mergeSpvInst(&typePointerOptions, 4, SpvOpTypePointer, UINT32_MAX, SpvStorageClassInput, typeVectorId);

    vertexAnnotation->signedness = signedness;
    vertexAnnotation->vecId = typeVectorId;
    module->code[vertexAnnotation->varOffset+1] = typePointerId;
}

static void mergeShaderCode(ShaderModule* module, IntArray* mergeCode, int mergeOffset) {
    size_t newCodeSize;
    uint32_t* newCode = memmerge(mergeCode->values, mergeCode->size * sizeof(int), module->code, module->codeSize,
                                 mergeOffset * sizeof(int), &newCodeSize);

    MEMFREE(module->code);
    module->code = newCode;
    module->codeSize = newCodeSize;
    IntArray_clear(mergeCode);
}

static void mergeCodesSorted(ShaderModule* module, ArrayList* mergeCodes) {
    while (mergeCodes->size > 0) {
        int index = 0;
        int maxPosition = -INT32_MAX;

        for (int i = 0; i < mergeCodes->size; i++) {
            MergeCode* mergeCode = mergeCodes->elements[i];
            if (mergeCode->position > maxPosition) {
                maxPosition = mergeCode->position;
                index = i;
            }
        }

        MergeCode* mergeCode = mergeCodes->elements[index];
        mergeShaderCode(module, &mergeCode->code, mergeCode->position);
        free(mergeCode);
        ArrayList_removeAt(mergeCodes, index);
    }

    MEMFREE(mergeCodes->elements);
}

static void removeSpvInstIndices(ShaderModule* module, IntArray* instIndices) {
    if (instIndices->size == 0) return;
    IntArray_sort(instIndices);

    uint32_t instLength;
    for (int i = instIndices->size-1, j; i >= 0; i--) {
        j = instIndices->values[i];
        instLength = SPV_INST_LENGTH(module->code[j]);

        size_t newCodeSize;
        uint32_t* newCode = memdel(module->code, module->codeSize, j * sizeof(int),
                                   instLength * sizeof(int), &newCodeSize);
        if (newCode) {
            MEMFREE(module->code);
            module->code = newCode;
            module->codeSize = newCodeSize;
        }
    }

    IntArray_clear(instIndices);
}

static void removePointSizeExport(ShaderModule* module, int decorOffset, IntArray* removeInsts, ArrayList* mergeCodes) {
    int index = fetchSpvInstIndex(module->code, module->codeSize, decorOffset, SpvOpTypeFloat, 2, 32, true);
    if (index == -1) return;

    const float constValue = 1.0f;
    SpvInstFilter filter = {{1, 3}, {module->code[index+1], *(uint32_t*)&constValue}, 2, 0};
    index = fetchSpvInstIndex2(module->code, module->codeSize, decorOffset, SpvOpConstant, &filter, true);
    if (index == -1) return;

    uint32_t constf32Id = module->code[index+2];
    uint32_t varId = module->code[decorOffset+1];

    int position = decorOffset;
    bool opStoreFound = false;
    do {
        index = fetchSpvInstIndex(module->code, module->codeSize, position, SpvOpStore, 1, varId, false);
        if (index != -1) {
            if (module->code[index+2] == constf32Id) {
                opStoreFound = true;
                break;
            }
            position = index + SPV_INST_LENGTH(module->code[index+0]);
        }
    }
    while (index != -1);

    if (!opStoreFound) return;

    int varOffset = fetchSpvInstIndex(module->code, module->codeSize, decorOffset, SpvOpVariable, 2, varId, true);
    if (varOffset == -1 || module->code[varOffset+3] != SpvStorageClassOutput) return;

    int pointerOffset = fetchSpvInstIndex(module->code, module->codeSize, decorOffset, SpvOpTypePointer, 1, module->code[varOffset+1], true);
    if (pointerOffset == -1) return;

    uint32_t baseTypeId = module->code[pointerOffset+3];
    MergeSpvInstOptions typePointerOptions = {module->code, module->codeSize, mergeCodes, pointerOffset, decorOffset, {{2, 3}, {SpvStorageClassPrivate, baseTypeId}, 2, 0}};
    uint32_t typePointerId = mergeSpvInst(&typePointerOptions, 4, SpvOpTypePointer, UINT32_MAX, SpvStorageClassPrivate, baseTypeId);

    module->code[varOffset+3] = SpvStorageClassPrivate;
    module->code[varOffset+1] = typePointerId;

    IntArray_add(removeInsts, decorOffset);
}

static void inspectVertexShaderCode(ShaderInspector* shaderInspector, ShaderModule* module, VertexAnnotation* vertexAnnotations, int vertexAnnotationCount) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;
    ArrayList mergeCodes = {0};
    IntArray removeInsts = {0};

    for (int i = CODE_START, size = module->codeSize / sizeof(uint32_t); i < size && opcode != SpvOpFunction && !IS_SPV_OP_TYPE(opcode);) {
        opcode = SPV_INST_OPCODE(module->code[i]);
        instLength = SPV_INST_LENGTH(module->code[i]);

        if (opcode == SpvOpCapability) {
            if (shaderInspector->checkClipDistance && module->code[i+1] == SpvCapabilityClipDistance) IntArray_add(&removeInsts, i);
        }
        else if (opcode == SpvOpDecorate && module->code[i+2] == SpvDecorationBuiltIn) {
            SpvBuiltIn builtIn = module->code[i+3];
            if (shaderInspector->checkClipDistance && builtIn == SpvBuiltInClipDistance) {
                IntArray_add(&removeInsts, i);
            }
            else if (shaderInspector->removePointSizeExport && builtIn == SpvBuiltInPointSize) {
                removePointSizeExport(module, i, &removeInsts, &mergeCodes);
            }
        }

        i += instLength;
    }

    removeSpvInstIndices(module, &removeInsts);

    if (shaderInspector->convertFormatScaled) {
        opcode = SpvOpNop;
        for (int i = CODE_START, j, size = module->codeSize / sizeof(uint32_t); i < size && opcode != SpvOpFunction && !IS_SPV_OP_TYPE(opcode);) {
            opcode = SPV_INST_OPCODE(module->code[i]);
            instLength = SPV_INST_LENGTH(module->code[i]);

            if (opcode == SpvOpDecorate && module->code[i+2] == SpvDecorationLocation) {
                uint32_t location = module->code[i+3];
                for (j = 0; j < vertexAnnotationCount; j++) {
                    VertexAnnotation* vertexAnnotation = &vertexAnnotations[j];
                    if (vertexAnnotation->location == location) {
                        uint32_t varId = module->code[i+1];

                        int index = fetchSpvInstIndex(module->code, module->codeSize, i, SpvOpVariable, 2, varId, true);
                        if (index != -1 && module->code[index+3] == SpvStorageClassInput) {
                            vertexAnnotation->varId = varId;
                            vertexAnnotation->varOffset = index;
                            convertInputVariableFormatScaled(module, &mergeCodes, i, vertexAnnotation);
                        }
                        break;
                    }
                }
            }

            i += instLength;
        }
    }

    if (mergeCodes.size > 0) {
        mergeCodesSorted(module, &mergeCodes);

        int functionStart = fetchSpvInstIndex(module->code, module->codeSize, CODE_START, SpvOpFunction, -1, 0, false);
        IntArray mergeCode = {0};
        for (int i = 0, j, position; i < vertexAnnotationCount; i++) {
            VertexAnnotation* vertexAnnotation = &vertexAnnotations[i];
            if (vertexAnnotation->varId == -1) continue;

            position = functionStart;
            do {
                j = fetchSpvInstIndex(module->code, module->codeSize, position, SpvOpLoad, 3, vertexAnnotation->varId, false);
                if (j != -1) {
                    uint32_t oldLoadId = module->code[j+2];
                    uint32_t newLoadId = module->code[MAX_ID_BOUND_OFFSET]++;
                    uint32_t resultTypeId = module->code[j+1];
                    instLength = SPV_INST_LENGTH(module->code[j+0]);
                    module->code[j+1] = vertexAnnotation->vecId;
                    module->code[j+2] = newLoadId;

                    position = j+instLength;
                    ADD_SPV_INST(&mergeCode, 4, vertexAnnotation->signedness ? SpvOpConvertSToF : SpvOpConvertUToF, resultTypeId, oldLoadId, newLoadId);
                    mergeShaderCode(module, &mergeCode, position);
                }
            }
            while (j != -1);
        }
    }
}

static void inspectFragmentShaderCode(ShaderInspector* shaderInspector, ShaderModule* vsModule, ShaderModule* fsModule) {
    ArrayList mergeCodes = {0};

    ArrayList vsVariables = {0};
    fetchSpvVariableOffsets(vsModule, SpvStorageClassOutput, &vsVariables);

    ArrayList fsVariables = {0};
    fetchSpvVariableOffsets(fsModule, SpvStorageClassInput, &fsVariables);

    for (int i = 0, j; i < fsVariables.size; i++) {
        SpvVariableOffsets* fsVariableOffsets = fsVariables.elements[i];
        uint32_t fsLocation = fsModule->code[fsVariableOffsets->decorate+3];
        uint32_t fsComponentCount = SPV_INST_OPCODE(fsModule->code[fsVariableOffsets->vector]) == SpvOpTypeVector ? fsModule->code[fsVariableOffsets->vector+3] : 1;

        for (j = 0; j < vsVariables.size; j++) {
            SpvVariableOffsets* vsVariableOffsets = vsVariables.elements[j];
            uint32_t vsLocation = vsModule->code[vsVariableOffsets->decorate+3];
            uint32_t vsComponentCount = SPV_INST_OPCODE(vsModule->code[vsVariableOffsets->vector]) == SpvOpTypeVector ? vsModule->code[vsVariableOffsets->vector+3] : 1;

            if (fsLocation == vsLocation && fsComponentCount != vsComponentCount) {
                uint32_t componentType = fsComponentCount > 1 ? fsModule->code[fsVariableOffsets->vector+2] : fsModule->code[fsVariableOffsets->vector+1];
                int componentTypeIndex = fsComponentCount > 1 ? fetchSpvOpTypeIndex(fsModule->code, fsModule->codeSize, fsVariableOffsets->decorate, 1, componentType) : fsVariableOffsets->vector;
                if (componentTypeIndex != -1) {
                    uint32_t componentCount = MAX(vsComponentCount, fsComponentCount);
                    int mergePosition = componentTypeIndex + SPV_INST_LENGTH(fsModule->code[componentTypeIndex]);

                    MergeSpvInstOptions typeVectorOptions = {fsModule->code, fsModule->codeSize, &mergeCodes, mergePosition, componentTypeIndex, {{2, 3}, {componentType, componentCount}, 2, fsVariableOffsets->variable}};
                    uint32_t typeVectorId = mergeSpvInst(&typeVectorOptions, 4, SpvOpTypeVector, UINT32_MAX, componentType, componentCount);

                    MergeSpvInstOptions typePointerOptions = {fsModule->code, fsModule->codeSize, &mergeCodes, typeVectorOptions.mergePosition, componentTypeIndex, {{2, 3}, {SpvStorageClassInput, typeVectorId}, 2, fsVariableOffsets->variable}};
                    uint32_t typePointerId = mergeSpvInst(&typePointerOptions, 4, SpvOpTypePointer, UINT32_MAX, SpvStorageClassInput, typeVectorId);

                    fsModule->code[fsVariableOffsets->variable+1] = typePointerId;
                }

                break;
            }
        }
    }

    ArrayList_free(&vsVariables);
    ArrayList_free(&fsVariables);

    mergeCodesSorted(fsModule, &mergeCodes);
}

static void removeImageBoundCheck(uint32_t* code, uint32_t codeSize) {
    uint32_t instLength;
    SpvOp opcode = SpvOpNop;
    bool inspectOpSelect = false;

    for (int i = CODE_START, size = codeSize / sizeof(uint32_t); i < size;) {
        opcode = SPV_INST_OPCODE(code[i]);
        instLength = SPV_INST_LENGTH(code[i]);

        if (opcode == SpvOpSampledImage || opcode == SpvOpImageFetch) {
            inspectOpSelect = true;
        }
        else if (opcode == SpvOpSelect && inspectOpSelect) {
            uint32_t* object1Id = &code[i+4];
            uint32_t* object2Id = &code[i+5];
            int index = fetchSpvInstIndex(code, codeSize, CODE_START, SpvOpConstantComposite, 2, *object2Id, true);

            uint32_t scalarId = index != -1 ? code[index+3] : *object2Id;
            index = fetchSpvInstIndex(code, codeSize, CODE_START, SpvOpConstant, 2, scalarId, true);
            if (index != -1) {
                uint32_t constValue = code[index+3];
                if (constValue == 0) *object2Id = *object1Id;
            }

            inspectOpSelect = false;
        }
        else if (opcode == SpvOpStore) {
            inspectOpSelect = false;
        }

        i += instLength;
    }
}

static SpvExecutionModel getSpvExecutionModel(const uint32_t* code, uint32_t sizeInBytes) {
    if (code[0] != SpvMagicNumber) return SpvExecutionModelMax;
    int index = fetchSpvInstIndex(code, sizeInBytes, CODE_START, SpvOpEntryPoint, -1, 0, true);
    return index != -1 ? code[index+1] : SpvExecutionModelMax;
}

ShaderInspector* ShaderInspector_create(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* supportedFeatures) {
    ShaderInspector* shaderInspector = calloc(1, sizeof(ShaderInspector));
    shaderInspector->checkClipDistance = supportedFeatures->shaderClipDistance == VK_FALSE; /* workaround for vertex explosion on Mali devices */

    VkPhysicalDeviceProperties deviceProperties = {0};
    vulkanWrapper.vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    bool isMaliDevice = strstr(deviceProperties.deviceName, "Mali") ? true : false;

    if (isMaliDevice) {
        VkFormatProperties formatProperties = {0};
        vulkanWrapper.vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_SSCALED, &formatProperties);
        shaderInspector->convertFormatScaled = !(formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT);
    }
    else shaderInspector->convertFormatScaled = true;

    shaderInspector->removeImageBoundCheck = isMaliDevice; /* workaround for black screen */
    shaderInspector->removePointSizeExport = true; /* workaround for graphical glitches on DXVK 2.4.1 (See Out Of Action game) */

    return shaderInspector;
}

VkResult ShaderInspector_createModule(ShaderInspector* shaderInspector, VkDevice device, const uint32_t* code, size_t codeSize, ShaderModule** ppModule) {
    ShaderModule* shaderModule = calloc(1, sizeof(ShaderModule));
    shaderModule->module = VK_NULL_HANDLE;
    SpvExecutionModel executionModel = getSpvExecutionModel(code, codeSize);

    if (shaderInspector->removeImageBoundCheck && (executionModel == SpvExecutionModelVertex || executionModel == SpvExecutionModelFragment)) {
        removeImageBoundCheck(code, codeSize);
    }

    VkResult result = VK_SUCCESS;
    if (executionModel == SpvExecutionModelVertex || executionModel == SpvExecutionModelFragment) {
        shaderModule->codeSize = codeSize;
        shaderModule->code = memdup(code, codeSize);
    }
    else {
        VkShaderModuleCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pCode = code;
        createInfo.codeSize = codeSize;
        result = vulkanWrapper.vkCreateShaderModule(device, &createInfo, NULL, &shaderModule->module);
    }

    *ppModule = shaderModule;
    return result;
}

VkResult ShaderInspector_inspectShaderStages(ShaderInspector* shaderInspector, VkDevice device, VkPipelineShaderStageCreateInfo* stageInfos, uint32_t stageCount, const VkPipelineVertexInputStateCreateInfo* vertexInputState) {
    VkResult result = VK_SUCCESS;
    ShaderModule* vertexShaderModule = NULL;

    for (int i = 0, j; i < stageCount; i++) {
        VkPipelineShaderStageCreateInfo* stageInfo = &stageInfos[i];
        ShaderModule* shaderModule = (ShaderModule*)stageInfo->module;
        if (!shaderModule) continue;

        if (!shaderModule->module) {
            if (stageInfo->stage == VK_SHADER_STAGE_VERTEX_BIT) {
                if (vertexInputState) {
                    VertexAnnotation vertexAnnotations[vertexInputState->vertexAttributeDescriptionCount];
                    int vertexAnnotationCount = 0;

                    for (j = 0; j < vertexInputState->vertexAttributeDescriptionCount; j++) {
                        VkVertexInputAttributeDescription* attributeDescription = (VkVertexInputAttributeDescription*)&vertexInputState->pVertexAttributeDescriptions[j];
                        if (isFormatScaled(attributeDescription->format)) {
                            attributeDescription->format = getFallbackFormat(attributeDescription->format);

                            VertexAnnotation* vertexAnnotation = &vertexAnnotations[vertexAnnotationCount++];
                            vertexAnnotation->location = attributeDescription->location;
                            vertexAnnotation->format = attributeDescription->format;
                            vertexAnnotation->vecId = -1;
                            vertexAnnotation->varId = -1;
                            vertexAnnotation->varOffset = -1;
                        }
                    }

                    inspectVertexShaderCode(shaderInspector, shaderModule, vertexAnnotations, vertexAnnotationCount);
                }

                vertexShaderModule = shaderModule;
            }
            else if (stageInfo->stage == VK_SHADER_STAGE_FRAGMENT_BIT && vertexShaderModule) {
                inspectFragmentShaderCode(shaderInspector, vertexShaderModule, shaderModule);
            }

            VkShaderModuleCreateInfo moduleInfo = {0};
            moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleInfo.pCode = shaderModule->code;
            moduleInfo.codeSize = shaderModule->codeSize;
            result = vulkanWrapper.vkCreateShaderModule(device, &moduleInfo, NULL, &shaderModule->module);
            if (result != VK_SUCCESS) break;
        }
    }

    for (int i = 0; i < stageCount; i++) {
        VkPipelineShaderStageCreateInfo* stageInfo = &stageInfos[i];
        ShaderModule* shaderModule = (ShaderModule*)stageInfo->module;
        if (!shaderModule) continue;

        MEMFREE(shaderModule->code);
        shaderModule->codeSize = 0;
        stageInfo->module = shaderModule->module;
    }

    return result;
}

bool isFormatScaled(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return true;
        default:
            return false;
    }
}
