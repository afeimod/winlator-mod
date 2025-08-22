#ifndef VORTEK_RESOURCE_MEMORY_H
#define VORTEK_RESOURCE_MEMORY_H

#include <android/hardware_buffer.h>

#include "vortek.h"

typedef enum ResourceMemoryType {
    RESOURCE_MEMORY_TYPE_AUTO,
    RESOURCE_MEMORY_TYPE_OPAQUE_FD,
    RESOURCE_MEMORY_TYPE_DMA_BUF,
    RESOURCE_MEMORY_TYPE_AHARDWAREBUFFER
} ResourceMemoryType;

typedef struct ResourceMemory {
    int fd;
    AHardwareBuffer* hardwareBuffer;
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize allocationSize;
} ResourceMemory;

extern ResourceMemory* ResourceMemory_allocate(VkContext* context, VkDevice device, VkMemoryAllocateInfo* memoryInfo);
extern void ResourceMemory_free(VkContext* context, VkDevice device, ResourceMemory* resourceMemory);

#endif
