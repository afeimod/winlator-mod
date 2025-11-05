#define HEADER_SIZE 8
#define DEVICE_NAME "Vortek (%s)"
#define ENABLE_VALIDATION_LAYER 0 // FIXME set to 0 and remove libVkLayer from jniLibs
#define DEBUG_MODE 0 // FIXME set to 0
#define MEMORY_POOL_MAX_SIZE 65536
#define SERVER_RING_BUFFER_SIZE 4194304
#define CLIENT_RING_BUFFER_SIZE 262144
#define VORTEK_SERVER_PATH "/data/data/com.winlator/files/rootfs/tmp/.vortek/V0"
#define VK_HANDLE_BYTE_COUNT 8
#define THREAD_POOL_NUM_THREADS 8

#include "winlator.h"

#ifdef __ANDROID__
#define VT_SERVER 1
#define VK_NO_PROTOTYPES 1
#endif

#define VT_CMD_ENQUEUE(cmdName, requestCode, batch, ...) \
    do { \
        int bufferSize = vt_sizeof_##cmdName(__VA_ARGS__); \
        ENSURE_ARRAY_CAPACITY(batch->size + bufferSize + HEADER_SIZE, batch->capacity, batch->buffer, 1); \
        char* chunk = batch->buffer + batch->size; \
        *(int*)(chunk + 0) = requestCode; \
        *(int*)(chunk + 4) = bufferSize; \
        vt_serialize_##cmdName(__VA_ARGS__, chunk + HEADER_SIZE); \
        batch->size += bufferSize + HEADER_SIZE; \
    } \
    while (0)

#ifdef VT_SERVER
#define VT_SERIALIZE_CMD(cmdName, ...) \
    int bufferSize = vt_sizeof_##cmdName(__VA_ARGS__); \
    char* outputBuffer = vt_alloc(&context->memoryPool, bufferSize); \
    vt_serialize_##cmdName(__VA_ARGS__, outputBuffer)
#else
#define VT_SERIALIZE_CMD(cmdName, ...) \
    int bufferSize = vt_sizeof_##cmdName(__VA_ARGS__); \
    char* outputBuffer = vt_alloc(&globalMemoryPool, bufferSize); \
    vt_serialize_##cmdName(__VA_ARGS__, outputBuffer)
#endif

#define VT_RETURN 1

#define VT_SEND_CHECKED(requestCode, ...) \
    do { \
        int bytesSent = vt_send(serverRing, requestCode, outputBuffer, bufferSize); \
        if (bytesSent != bufferSize) { \
            VT_CALL_UNLOCK(); \
            return __VA_OPT__(VK_ERROR_DEVICE_LOST); \
        } \
    } \
    while (0)

#define VT_RECV_CHECKED(...) \
    char* inputBuffer = NULL; \
    int result; \
    do { \
        result = vt_recv(clientRing, &inputBuffer, NULL, &globalMemoryPool); \
        if (result == VK_ERROR_DEVICE_LOST) { \
            VT_CALL_UNLOCK(); \
            return __VA_OPT__(VK_ERROR_DEVICE_LOST); \
        } \
    } \
    while (0)

#define IS_DESCRIPTOR_IMAGE_INFO(type) (type == VK_DESCRIPTOR_TYPE_SAMPLER || type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE || type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
#define IS_DESCRIPTOR_BUFFER_INFO(type) (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
#define IS_DESCRIPTOR_TEXEL_BUFFER_VIEW(type) (type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)

#ifndef VORTEK_H
#define VORTEK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

#include "vulkan/vulkan.h"
#include "request_codes.h"
#include "vk_object.h"
#include "arrays.h"
#include "events.h"
#include "time_utils.h"
#include "socket_utils.h"
#include "ring_buffer.h"
#include "thread_pool.h"

typedef struct MemoryPool {
    void* data;
    int size;
    ArrayList allocationList;
} MemoryPool;

typedef struct VkContext VkContext;

#ifdef VT_SERVER

#include <jni.h>
#include <android/log.h>

#include "resource_memory.h"
#include "shader_inspector.h"

typedef struct JMethods {
    JavaVM* jvm;
    JNIEnv* env;
    jobject obj;
    jmethodID getWindowWidth;
    jmethodID getWindowHeight;
    jmethodID getWindowHardwareBuffer;
    jmethodID updateWindowContent;
} JMethods;

#else // VT_SERVER

typedef struct MappedMemory {
    void* data;
    int allocationSize;
    int size;
} MappedMemory;

typedef struct CommandBatch {
    char* buffer;
    int capacity;
    int size;
} CommandBatch;

extern bool vortekInitOnce();
extern int serverFd;
extern uint16_t maxClientRequestId;
extern MemoryPool globalMemoryPool;
extern RingBuffer* serverRing;
extern RingBuffer* clientRing;
#endif

static inline void* findNextVkStructure(void* pNext, VkStructureType type) {
    while (pNext) {
        VkBaseOutStructure* curr = pNext;
        if (curr->sType == type) return pNext;
        pNext = curr->pNext;
    }

    return NULL;
}

static inline void* invertVkStructuresChain(void* pNext) {
    void* pPrev = NULL;

    while (pNext) {
        VkBaseOutStructure* curr = pNext;
        pNext = curr->pNext;
        curr->pNext = pPrev;
        pPrev = curr;
    }

    return pPrev;
}

static inline void* removeNextVkStructure(void* pNext, VkStructureType type) {
    VkBaseOutStructure* prev = NULL;
    void* pFirst = pNext;

    while (pNext) {
        VkBaseOutStructure* curr = pNext;
        pNext = curr->pNext;
        if (curr->sType == type) {
            if (prev) {
                prev->pNext = pNext;
            }
            else pFirst = pNext;
            break;
        }
        prev = curr;
    }

    return pFirst;
}

static inline void* vt_alloc(MemoryPool* memoryPool, int size) {
    bool isFull = (memoryPool->size + size) >= MEMORY_POOL_MAX_SIZE || !memoryPool->data;
    void* chunk;
    if (isFull) {
        chunk = malloc(size);
        ArrayList_add(&memoryPool->allocationList, chunk);
    }
    else {
        chunk = memoryPool->data + memoryPool->size;
        memoryPool->size += size;
    }

    memset(chunk, 0, size);
    return chunk;
}

static inline void vt_free(MemoryPool* memoryPool) {
    if (!memoryPool) return;
    memoryPool->size = 0;

    for (int i = memoryPool->allocationList.size-1; i >= 0; i--) {
        MEMFREE(memoryPool->allocationList.elements[i]);
        ArrayList_removeAt(&memoryPool->allocationList, i);
    }
}

static inline int vt_send(RingBuffer* ring, int requestCode, void* data, int size) {
#ifndef VT_SERVER
    if (size >= SERVER_RING_BUFFER_SIZE) {
        const uint16_t requestId = maxClientRequestId++;
        char header[HEADER_SIZE];
        *(int*)(header + 0) = PACK16(REQUEST_CODE_SEND_EXTRA_DATA, requestId);
        *(int*)(header + 4) = size;

        int bytesSent = sock_write(serverFd, header, HEADER_SIZE) ;
        if (bytesSent != HEADER_SIZE) return 0;

        bytesSent = sock_write(serverFd, data, size);
        if (bytesSent != size) return 0;

        *(int*)(header + 0) = PACK16(requestCode, requestId);
        *(int*)(header + 4) = 0;
        bool result = RingBuffer_write(ring, header, HEADER_SIZE);
        if (!result) return 0;

        return size;
    }
#endif

    char header[HEADER_SIZE];
    *(int*)(header + 0) = requestCode;
    *(int*)(header + 4) = size;

    bool result = RingBuffer_write(ring, header, HEADER_SIZE);
    if (!result) return 0;

    if (size > 0) {
        result = RingBuffer_write(ring, data, size);
        if (!result) return 0;
    }

    return size;
}

static inline int vt_recv(RingBuffer* ring, char** inputBuffer, int* bufferSize, MemoryPool* memoryPool) {
    char header[HEADER_SIZE];
    bool result = RingBuffer_read(ring, header, HEADER_SIZE);
    if (!result) return VK_ERROR_DEVICE_LOST;

    int requestCode = *(int*)(header + 0);
    int size = *(int*)(header + 4);

    if (size > 0) {
        *inputBuffer = vt_alloc(memoryPool, size);
        result = RingBuffer_read(ring, *inputBuffer, size);
        if (!result) return VK_ERROR_DEVICE_LOST;
    }

    if (bufferSize) *bufferSize = size;
    return requestCode;
}

#endif