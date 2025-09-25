#include "async_pipeline_creator.h"
#include "vortek_serializer.h"
#include "vulkan_helper.h"

typedef struct PipelineCreateRequest {
    VkDevice device;
    VkPipelineCache pipelineCache;
    PipelineType type;
    uint32_t pipelineCount;
    void* pipelineInfos;
    ShaderInspector* shaderInspector;
    MemoryPool memoryPool;
    int notifyFd;
} PipelineCreateRequest;

static void pipelineCreateThread(void* param) {
    PipelineCreateRequest* pipelineCreateRequest = param;
    VkPipeline pipelines[pipelineCreateRequest->pipelineCount];
    VkResult result = VK_SUCCESS;

    if (pipelineCreateRequest->type == PIPELINE_TYPE_GRAPHICS) {
        VkGraphicsPipelineCreateInfo* createInfos = pipelineCreateRequest->pipelineInfos;

        for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
            ShaderInspector_inspectShaderStages(pipelineCreateRequest->shaderInspector, pipelineCreateRequest->device, (VkPipelineShaderStageCreateInfo*)createInfos[i].pStages, createInfos[i].stageCount, createInfos[i].pVertexInputState);
        }

        result = vulkanWrapper.vkCreateGraphicsPipelines(pipelineCreateRequest->device, pipelineCreateRequest->pipelineCache, pipelineCreateRequest->pipelineCount, createInfos, NULL, pipelines);
    }
    else if (pipelineCreateRequest->type == PIPELINE_TYPE_COMPUTE) {
        VkComputePipelineCreateInfo* createInfos = pipelineCreateRequest->pipelineInfos;

        for (int i = 0; i < pipelineCreateRequest->pipelineCount; i++) {
            ShaderInspector_inspectShaderStages(pipelineCreateRequest->shaderInspector, pipelineCreateRequest->device, (VkPipelineShaderStageCreateInfo*)&createInfos[i].stage, 1, NULL);
        }

        result = vulkanWrapper.vkCreateComputePipelines(pipelineCreateRequest->device, pipelineCreateRequest->pipelineCache, pipelineCreateRequest->pipelineCount, createInfos, NULL, pipelines);
    }

    vt_free(&pipelineCreateRequest->memoryPool);

    int bufferSize = sizeof(VkResult) + pipelineCreateRequest->pipelineCount * VK_HANDLE_BYTE_COUNT;
    char outputBuffer[bufferSize];

    *(int*)(outputBuffer + 0) = result;
    for (int i = 0, j = sizeof(VkResult); i < pipelineCreateRequest->pipelineCount; i++, j += VK_HANDLE_BYTE_COUNT) {
        *(uint64_t*)(outputBuffer + j) = (uint64_t)pipelines[i];
    }

    write(pipelineCreateRequest->notifyFd, outputBuffer, bufferSize);

    CLOSEFD(pipelineCreateRequest->notifyFd);
    MEMFREE(pipelineCreateRequest->pipelineInfos);
    MEMFREE(pipelineCreateRequest);
}

void AsyncPipelineCreator_create(VkContext* context, PipelineType type) {
    PipelineCreateRequest* pipelineCreateRequest = calloc(1, sizeof(PipelineCreateRequest));
    pipelineCreateRequest->type = type;
    pipelineCreateRequest->shaderInspector = context->shaderInspector;

    uint64_t deviceId = 0;
    uint64_t pipelineCacheId = 0;

    if (type == PIPELINE_TYPE_GRAPHICS) {
        vt_unserialize_vkCreateGraphicsPipelines((VkDevice)&deviceId, (VkPipelineCache)&pipelineCacheId, &pipelineCreateRequest->pipelineCount, NULL, NULL, NULL, context->inputBuffer, NULL);

        VkGraphicsPipelineCreateInfo* createInfos = calloc(pipelineCreateRequest->pipelineCount, sizeof(VkGraphicsPipelineCreateInfo));
        vt_unserialize_vkCreateGraphicsPipelines(VK_NULL_HANDLE, VK_NULL_HANDLE, NULL, createInfos, NULL, NULL, context->inputBuffer, &pipelineCreateRequest->memoryPool);
        pipelineCreateRequest->pipelineInfos = createInfos;
    }
    else if (type == PIPELINE_TYPE_COMPUTE) {
        vt_unserialize_vkCreateComputePipelines((VkDevice)&deviceId, (VkPipelineCache)&pipelineCacheId, &pipelineCreateRequest->pipelineCount, NULL, NULL, NULL, context->inputBuffer, NULL);

        VkComputePipelineCreateInfo* createInfos = calloc(pipelineCreateRequest->pipelineCount, sizeof(VkComputePipelineCreateInfo));
        vt_unserialize_vkCreateComputePipelines(VK_NULL_HANDLE, VK_NULL_HANDLE, NULL, createInfos, NULL, NULL, context->inputBuffer, &pipelineCreateRequest->memoryPool);
        pipelineCreateRequest->pipelineInfos = createInfos;
    }

    pipelineCreateRequest->device = VkObject_fromId(deviceId);
    pipelineCreateRequest->pipelineCache = VkObject_fromId(pipelineCacheId);

    int fds[2] = {0};
    char success = pipe(fds) == 0 ? 1 : 0;
    send_fds(context->clientFd, fds, 1, &success, 1);

    if (success) {
        pipelineCreateRequest->notifyFd = fds[1];
        CLOSEFD(fds[0]);
        ThreadPool_run(context->threadPool, pipelineCreateThread, pipelineCreateRequest);
    }
    else {
        MEMFREE(pipelineCreateRequest->pipelineInfos);
        MEMFREE(pipelineCreateRequest);
    }
}