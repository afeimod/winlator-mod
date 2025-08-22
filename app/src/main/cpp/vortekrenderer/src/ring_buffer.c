#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/mman.h>

#include "ring_buffer.h"
#include "time_utils.h"
#include "events.h"

#define STRUCT_OFFSETS(x) \
    struct Offsets { \
        __attribute__((aligned(x))) uint32_t head; \
        __attribute__((aligned(x))) uint32_t tail; \
        __attribute__((aligned(x))) uint32_t status; \
        __attribute__((aligned(x))) void* buffer; \
    }

#ifdef __ANDROID__
#include <android/log.h>
#define debug_printf(...) __android_log_print(ANDROID_LOG_DEBUG, "System.out", __VA_ARGS__)
#else
#define debug_printf(...) fprintf(stderr, __VA_ARGS__)
#endif

#define COND_WAIT(cond) \
    uint32_t busyWaitIter = 0; \
    do { \
        if (cond) break; \
        busyWait(&busyWaitIter); \
        if (RingBuffer_hasStatus(ring, RING_STATUS_EXIT)) return false; \
    } \
    while (1)

static void RingBuffer_setHead(RingBuffer* ring, uint32_t head) {
    atomic_store_explicit(ring->head, head, memory_order_release);
}

static uint32_t RingBuffer_getHead(RingBuffer* ring) {
    return atomic_load_explicit(ring->head, memory_order_acquire);
}

static void RingBuffer_setTail(RingBuffer* ring, uint32_t tail) {
    return atomic_store_explicit(ring->tail, tail, memory_order_release);
}

static uint32_t RingBuffer_getTail(RingBuffer* ring) {
    return atomic_load_explicit(ring->tail, memory_order_acquire);
}

void RingBuffer_setStatus(RingBuffer* ring, uint32_t status) {
    atomic_fetch_or_explicit(ring->status, status, memory_order_seq_cst);
}

void RingBuffer_unsetStatus(RingBuffer* ring, uint32_t status) {
    atomic_fetch_and_explicit(ring->status, ~status, memory_order_seq_cst);
}

bool RingBuffer_hasStatus(RingBuffer* ring, uint32_t status) {
    return (atomic_load_explicit(ring->status, memory_order_seq_cst) & status);
}

RingBuffer* RingBuffer_create(int shmFd, uint32_t bufferSize) {
    RingBuffer* ring = calloc(1, sizeof(RingBuffer));

    STRUCT_OFFSETS(64);

    int shmSize = RingBuffer_getSHMemSize(bufferSize);
    void* sharedData = mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (sharedData == MAP_FAILED) return NULL;
    memset(sharedData, 0, shmSize);

    ring->head = sharedData + offsetof(struct Offsets, head);
    ring->tail = sharedData + offsetof(struct Offsets, tail);
    ring->status = sharedData + offsetof(struct Offsets, status);
    ring->buffer = sharedData + offsetof(struct Offsets, buffer);
    ring->bufferSize = bufferSize;

    RingBuffer_setStatus(ring, RING_STATUS_IDLE);
    return ring;
}

uint32_t RingBuffer_size(RingBuffer* ring) {
    return (RingBuffer_getTail(ring) - RingBuffer_getHead(ring)) & (ring->bufferSize - 1);
}

uint32_t RingBuffer_freeSpace(RingBuffer* ring) {
    return ring->bufferSize - RingBuffer_size(ring);
}

bool RingBuffer_read(RingBuffer* ring, void* data, uint32_t size) {
    if (size > ring->bufferSize) {
        debug_printf("ring: buffer overflow on read (%d/%d)\n", size, ring->bufferSize);
        return false;
    }

    COND_WAIT(RingBuffer_size(ring) >= size);
    uint32_t head = RingBuffer_getHead(ring);

    if ((head + size) <= ring->bufferSize) {
        memcpy(data, ring->buffer + head, size);
    }
    else {
        uint32_t start = ring->bufferSize - head;
        memcpy(data, ring->buffer + head, start);
        memcpy(data + start, ring->buffer, size - start);
    }

    RingBuffer_setHead(ring, (head + size) & (ring->bufferSize - 1));
    return true;
}

bool RingBuffer_write(RingBuffer* ring, const void* data, uint32_t size) {
    if (size > ring->bufferSize) {
        debug_printf("ring: buffer overflow on write (%d/%d)\n", size, ring->bufferSize);
        return false;
    }

    COND_WAIT(RingBuffer_freeSpace(ring) >= size);
    uint32_t tail = RingBuffer_getTail(ring);

    if ((tail + size) <= ring->bufferSize) {
        memcpy(ring->buffer + tail, data, size);
    }
    else {
        uint32_t start = ring->bufferSize - tail;
        memcpy(ring->buffer + tail, data, start);
        memcpy(ring->buffer, data + start, size - start);
    }

    RingBuffer_setTail(ring, (tail + size) & (ring->bufferSize - 1));
    return true;
}

uint32_t RingBuffer_getSHMemSize(uint32_t bufferSize) {
    STRUCT_OFFSETS(64);

    return bufferSize + offsetof(struct Offsets, buffer);
}

void RingBuffer_free(RingBuffer* ring) {
    if (!ring) return;

    if (ring->sharedData) {
        RingBuffer_setStatus(ring, RING_STATUS_EXIT);
        munmap(ring->sharedData, RingBuffer_getSHMemSize(ring->bufferSize));
        ring->sharedData = NULL;
    }

    free(ring);
}