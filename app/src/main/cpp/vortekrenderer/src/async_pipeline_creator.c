#include "async_pipeline_creator.h"
#include "vortek_serializer.h"
#include "string_utils.h"
#include "vulkan_helper.h"

static bool markedToDestroy = true;

typedef struct PipelineCreateRequest {
    AsyncPipelineCreator* asyncPipelineCreator;
    VkDevice device;
    VkPipelineCache pipelineCache;
    PipelineType type;
    uint32_t pipelineCount;
    void* pipelineInfos;
    ShaderInspector* shaderInspector;
    ArrayList pipelines;
    MemoryPool memoryPool;
} PipelineCreateRequest;

static void markVkObjectAsBusy(AsyncPipelineCreator* asyncPipelineCreator, VkObjectType type, uint64_t id, void* handle) {
    if (!handle) return;
    pthread_mutex_lock(&asyncPipelineCreator->mutex);

    VkObject* object = calloc(1, sizeof(VkObject));
    object->id = id;
    object->type = type;
    object->handle = handle;
    ArrayList_add(&asyncPipelineCreator->busyObjects, object);

    pthread_mutex_unlock(&asyncPipelineCreator->mutex);
}

static void destroyMarkedVkObjects(AsyncPipelineCreator* asyncPipelineCreator, VkDevice device, uint64_t id) {
    pthread_mutex_lock(&asyncPipelineCreator->mutex);

    for (int i = asyncPipelineCreator->busyObjects.size-1; i >= 0; i--) {
        VkObject* object = asyncPipelineCreator->busyObjects.elements[i];
        if (object->id == id) {
            if (object->tag == &markedToDestroy) destroyVkObject(object->type, device, object->handle);
            MEMFREE(object);
            ArrayList_removeAt(&asyncPipelineCreator->busyObjects, i);
        }
    }

    pthread_mutex_unlock(&asyncPipelineCreator->mutex);
}

static void assignPipelineHandles(PipelineCreateRequest* pipelineCreateRequest, VkPipeline* pipelines) {
    for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
        PipelineHandle* pipelineHandle = pipelineCreateRequest->pipelines.elements[i];
        pthread_mutex_lock(&pipelineHandle->mutex);

        pipelineHandle->pipeline = pipelines[i];
        pipelineHandle->ready = true;

        pthread_cond_signal(&pipelineHandle->cond);
        pthread_mutex_unlock(&pipelineHandle->mutex);
    }
}

static void createGraphicsPipelines(PipelineCreateRequest* pipelineCreateRequest) {
    VkGraphicsPipelineCreateInfo* createInfos = pipelineCreateRequest->pipelineInfos;

    for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
        ShaderInspector_inspectShaderStages(pipelineCreateRequest->shaderInspector, pipelineCreateRequest->device, (VkPipelineShaderStageCreateInfo*)createInfos[i].pStages, createInfos[i].stageCount, createInfos[i].pVertexInputState);
    }

    VkPipeline pipelines[pipelineCreateRequest->pipelineCount];
    vulkanWrapper.vkCreateGraphicsPipelines(pipelineCreateRequest->device, pipelineCreateRequest->pipelineCache, pipelineCreateRequest->pipelineCount, createInfos, NULL, pipelines);

    destroyMarkedVkObjects(pipelineCreateRequest->asyncPipelineCreator, pipelineCreateRequest->device, (uint64_t)pipelineCreateRequest);
    assignPipelineHandles(pipelineCreateRequest, pipelines);
    vt_free(&pipelineCreateRequest->memoryPool);
}

static void createComputePipelines(PipelineCreateRequest* pipelineCreateRequest) {
    VkComputePipelineCreateInfo* createInfos = pipelineCreateRequest->pipelineInfos;

    for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
        ShaderInspector_inspectShaderStages(pipelineCreateRequest->shaderInspector, pipelineCreateRequest->device, (VkPipelineShaderStageCreateInfo*)&createInfos[i].stage, 1, NULL);
    }

    VkPipeline pipelines[pipelineCreateRequest->pipelineCount];
    vulkanWrapper.vkCreateComputePipelines(pipelineCreateRequest->device, pipelineCreateRequest->pipelineCache, pipelineCreateRequest->pipelineCount, createInfos, NULL, pipelines);

    destroyMarkedVkObjects(pipelineCreateRequest->asyncPipelineCreator, pipelineCreateRequest->device, (uint64_t)pipelineCreateRequest);
    assignPipelineHandles(pipelineCreateRequest, pipelines);
    vt_free(&pipelineCreateRequest->memoryPool);
}

static void pipelineCreateThread(void* param) {
    PipelineCreateRequest* pipelineCreateRequest = param;

    if (pipelineCreateRequest->type == PIPELINE_TYPE_GRAPHICS) {
        createGraphicsPipelines(pipelineCreateRequest);
    }
    else if (pipelineCreateRequest->type == PIPELINE_TYPE_COMPUTE) {
        createComputePipelines(pipelineCreateRequest);
    }

    MEMFREE(pipelineCreateRequest->pipelines.elements);
    MEMFREE(pipelineCreateRequest->pipelineInfos);
    MEMFREE(pipelineCreateRequest);
}

VkPipeline AsyncPipelineCreator_getVkHandle(PipelineHandle* pipelineHandle) {
    if (pipelineHandle->ready) return pipelineHandle->pipeline;

    pthread_mutex_lock(&pipelineHandle->mutex);
    while (!pipelineHandle->ready) pthread_cond_wait(&pipelineHandle->cond, &pipelineHandle->mutex);
    pthread_mutex_unlock(&pipelineHandle->mutex);

    return pipelineHandle->pipeline;
}

void AsyncPipelineCreator_create(AsyncPipelineCreator* asyncPipelineCreator, RingBuffer* clientRing, ShaderInspector* shaderInspector, ThreadPool* threadPool, PipelineType type, char* inputBuffer) {
    PipelineCreateRequest* pipelineCreateRequest = calloc(1, sizeof(PipelineCreateRequest));
    pipelineCreateRequest->asyncPipelineCreator = asyncPipelineCreator;
    pipelineCreateRequest->type = type;
    pipelineCreateRequest->shaderInspector = shaderInspector;

    uint64_t deviceId = 0;
    uint64_t pipelineCacheId = 0;
    uint64_t pipelineCreateRequestId = (uint64_t)pipelineCreateRequest;

    if (type == PIPELINE_TYPE_GRAPHICS) {
        vt_unserialize_vkCreateGraphicsPipelines((VkDevice)&deviceId, (VkPipelineCache)&pipelineCacheId, &pipelineCreateRequest->pipelineCount, NULL, NULL, NULL, inputBuffer, NULL);

        VkGraphicsPipelineCreateInfo* createInfos = calloc(pipelineCreateRequest->pipelineCount, sizeof(VkGraphicsPipelineCreateInfo));
        vt_unserialize_vkCreateGraphicsPipelines(VK_NULL_HANDLE, VK_NULL_HANDLE, NULL, createInfos, NULL, NULL, inputBuffer, &pipelineCreateRequest->memoryPool);
        pipelineCreateRequest->pipelineInfos = createInfos;

        for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
            markVkObjectAsBusy(asyncPipelineCreator, VK_OBJECT_TYPE_RENDER_PASS, pipelineCreateRequestId, createInfos[i].renderPass);

            for (int j = 0; j < createInfos[i].stageCount; j++) {
                markVkObjectAsBusy(asyncPipelineCreator, VK_OBJECT_TYPE_SHADER_MODULE, pipelineCreateRequestId, createInfos[i].pStages[j].module);
            }
        }
    }
    else if (type == PIPELINE_TYPE_COMPUTE) {
        vt_unserialize_vkCreateComputePipelines((VkDevice)&deviceId, (VkPipelineCache)&pipelineCacheId, &pipelineCreateRequest->pipelineCount, NULL, NULL, NULL, inputBuffer, NULL);

        VkComputePipelineCreateInfo* createInfos = calloc(pipelineCreateRequest->pipelineCount, sizeof(VkComputePipelineCreateInfo));
        vt_unserialize_vkCreateComputePipelines(VK_NULL_HANDLE, VK_NULL_HANDLE, NULL, createInfos, NULL, NULL, inputBuffer, &pipelineCreateRequest->memoryPool);
        pipelineCreateRequest->pipelineInfos = createInfos;

        for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
            markVkObjectAsBusy(asyncPipelineCreator, VK_OBJECT_TYPE_SHADER_MODULE, pipelineCreateRequestId, createInfos[i].stage.module);
        }
    }

    pipelineCreateRequest->device = VkObject_fromId(deviceId);
    pipelineCreateRequest->pipelineCache = VkObject_fromId(pipelineCacheId);

    int bufferSize = pipelineCreateRequest->pipelineCount * VK_HANDLE_BYTE_COUNT;
    char outputBuffer[bufferSize];

    for (int i = 0, j = 0; i < pipelineCreateRequest->pipelineCount; i++, j += VK_HANDLE_BYTE_COUNT) {
        PipelineHandle* pipelineHandle = malloc(sizeof(PipelineHandle));
        pipelineHandle->pipeline = VK_NULL_HANDLE;
        pipelineHandle->ready = false;
        pthread_mutex_init(&pipelineHandle->mutex, NULL);
        pthread_cond_init(&pipelineHandle->cond, NULL);
        ArrayList_add(&pipelineCreateRequest->pipelines, pipelineHandle);

        vt_serialize_VkPipeline((VkPipeline)pipelineHandle, outputBuffer + j);
    }

    ThreadPool_run(threadPool, pipelineCreateThread, pipelineCreateRequest);
    vt_send(clientRing, VK_SUCCESS, outputBuffer, bufferSize);
}

void destroyVkObjectIfNotBusy(AsyncPipelineCreator* asyncPipelineCreator, VkObjectType type, VkDevice device, void* handle) {
    if (!handle) return;
    pthread_mutex_lock(&asyncPipelineCreator->mutex);

    bool busy = false;
    for (int i = 0; i < asyncPipelineCreator->busyObjects.size; i++) {
        VkObject* object = asyncPipelineCreator->busyObjects.elements[i];
        if (object->handle == handle) {
            object->tag = &markedToDestroy;
            busy = true;
            break;
        }
    }

    pthread_mutex_unlock(&asyncPipelineCreator->mutex);
    if (!busy) destroyVkObject(type, device, handle);
}