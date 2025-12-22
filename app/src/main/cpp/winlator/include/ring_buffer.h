#ifndef WINLATOR_RING_BUFFER_H
#define WINLATOR_RING_BUFFER_H

#include <stdatomic.h>
#include <stdint.h>

typedef struct RingBuffer {
    atomic_uint* head;
    atomic_uint* tail;
    atomic_uint* status;
    void* buffer;
    void* sharedData;
    uint32_t bufferSize;
} RingBuffer;

#define RING_STATUS_IDLE (1u<<0)
#define RING_STATUS_EXIT (1u<<1)
#define RING_STATUS_WAIT (1u<<2)

extern void RingBuffer_setStatus(RingBuffer* ring, uint32_t status);
extern void RingBuffer_unsetStatus(RingBuffer* ring, uint32_t status);
extern bool RingBuffer_hasStatus(RingBuffer* ring, uint32_t status);
extern RingBuffer* RingBuffer_create(int shmFd, uint32_t bufferSize);
extern uint32_t RingBuffer_size(RingBuffer* ring);
extern uint32_t RingBuffer_freeSpace(RingBuffer* ring);
extern bool RingBuffer_read(RingBuffer* ring, void* data, uint32_t size);
extern bool RingBuffer_write(RingBuffer* ring, const void *data, uint32_t size);
extern uint32_t RingBuffer_getSHMemSize(uint32_t bufferSize);
extern void RingBuffer_free(RingBuffer* ring);

#endif