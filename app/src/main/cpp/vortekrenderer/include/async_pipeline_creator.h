#ifndef VORTEK_ASYNC_PIPELINE_CREATOR_H
#define VORTEK_ASYNC_PIPELINE_CREATOR_H

#include "vortek.h"

typedef enum PipelineType {
    PIPELINE_TYPE_GRAPHICS,
    PIPELINE_TYPE_COMPUTE
} PipelineType;

typedef struct PipelineHandle {
    VkPipeline pipeline;
    bool ready;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} PipelineHandle;

typedef struct AsyncPipelineCreator {
    ArrayList busyObjects;
    pthread_mutex_t mutex;
} AsyncPipelineCreator;

extern VkPipeline AsyncPipelineCreator_getVkHandle(PipelineHandle* pipelineHandle);
extern void AsyncPipelineCreator_create(AsyncPipelineCreator* asyncPipelineCreator, RingBuffer* clientRing, ShaderInspector* shaderInspector, ThreadPool* threadPool, PipelineType type, char* inputBuffer);
extern void destroyVkObjectIfNotBusy(AsyncPipelineCreator* asyncPipelineCreator, VkObjectType type, VkDevice device, void* handle);

#endif