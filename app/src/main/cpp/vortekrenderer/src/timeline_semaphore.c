#include <sys/eventfd.h>

#include "timeline_semaphore.h"
#include "vortek_serializer.h"
#include "string_utils.h"
#include "vulkan_helper.h"

typedef struct WaitSemaphoresRequest {
    int notifyFd;
    char* inputBuffer;
} WaitSemaphoresRequest;

static void waitSemaphoresThread(void* param) {
    WaitSemaphoresRequest* waitSemaphoresRequest = param;

    uint64_t deviceId;
    VkSemaphoreWaitInfo waitInfo = {0};
    uint64_t timeout;

    MemoryPool memoryPool = {0};
    vt_unserialize_vkWaitSemaphores((VkDevice)&deviceId, &waitInfo, &timeout, waitSemaphoresRequest->inputBuffer, &memoryPool);
    VkDevice device = VkObject_fromId(deviceId);

    VkResult result = vulkanWrapper.vkWaitSemaphores(device, &waitInfo, timeout);

    uint64_t value = result == VK_SUCCESS ? 1 : (result == VK_ERROR_DEVICE_LOST ? 2 : 3);
    write(waitSemaphoresRequest->notifyFd, &value, sizeof(uint64_t));
    CLOSEFD(waitSemaphoresRequest->notifyFd);

    vt_free(&memoryPool);
}

void TimelineSemaphore_asyncWait(int clientFd, ThreadPool* threadPool, char* inputBuffer, int inputBufferSize) {
    WaitSemaphoresRequest* waitSemaphoresRequest = calloc(1, sizeof(WaitSemaphoresRequest));
    waitSemaphoresRequest->inputBuffer = memdup(inputBuffer, inputBufferSize);

    int fd = eventfd(0, 0);
    waitSemaphoresRequest->notifyFd = fd;

    send_fds(clientFd, &fd, 1, NULL, 0);
    ThreadPool_run(threadPool, waitSemaphoresThread, waitSemaphoresRequest);
}