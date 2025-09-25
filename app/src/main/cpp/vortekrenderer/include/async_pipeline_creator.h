#ifndef VORTEK_ASYNC_PIPELINE_CREATOR_H
#define VORTEK_ASYNC_PIPELINE_CREATOR_H

#include "vortek.h"

typedef enum PipelineType {
    PIPELINE_TYPE_GRAPHICS,
    PIPELINE_TYPE_COMPUTE
} PipelineType;

extern void AsyncPipelineCreator_create(VkContext* context, PipelineType type);

#endif