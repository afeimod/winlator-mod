#ifndef WINLATOR_DMA_UTILS_H
#define WINLATOR_DMA_UTILS_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/dma-heap.h>
#include <errno.h>

#include "ion/ion.h"
#include "ion/ion_4.19.h"

#define ION_SYSTEM_HEAP_ID_MASK (1u << 25)

static int safe_ioctl(int fd, uint64_t request, void *arg) {
    int res;

    do {
        res = ioctl(fd, request, arg);
    }
    while (res == -1 && (errno == EINTR || errno == EAGAIN));
    return res;
}

static inline int dmabuf_alloc(uint64_t size) {
    int res;
    int dma_fd = open("/dev/dma_heap/system-uncached", O_RDONLY);
    if (dma_fd < 0) dma_fd = open("/dev/dma_heap/system", O_RDONLY);

    if (dma_fd >= 0) {
        struct dma_heap_allocation_data alloc_data = {
            .len = size,
            .fd_flags = O_RDWR | O_CLOEXEC
        };

        res = safe_ioctl(dma_fd, DMA_HEAP_IOCTL_ALLOC, &alloc_data);
        close(dma_fd);

        if (res) return -1;
        return alloc_data.fd;
    }
    else {
        int ion_fd = open("/dev/ion", O_RDONLY);
        if (ion_fd < 0) return -1;

        struct ion_handle_data free = {0};
        free.handle = 0;
        if (safe_ioctl(ion_fd, ION_IOC_FREE, &free) >= 0 || errno != ENOTTY) {
            struct ion_allocation_data alloc_data = {
                .len = size,
                .align = 4096,
                .heap_id_mask = ION_SYSTEM_HEAP_ID_MASK,
                .flags = 0,
                .handle = -1
            };

            res = safe_ioctl(ion_fd, ION_IOC_ALLOC, &alloc_data);
            if (res) {
                close(ion_fd);
                return -1;
            }

            struct ion_fd_data share = {
                .handle = alloc_data.handle,
                .fd = -1,
            };

            res = safe_ioctl(ion_fd, ION_IOC_SHARE, &share);
            if (res) {
                close(ion_fd);
                return -1;
            }

            free.handle = alloc_data.handle;
            res = safe_ioctl(ion_fd, ION_IOC_FREE, &free);
            close(ion_fd);

            if (res) return -1;
            return share.fd;
        }
        else {
            struct ion_new_allocation_data alloc_data = {
                .len = size,
                .heap_id_mask = ION_SYSTEM_HEAP_ID_MASK,
                .flags = 0,
                .fd = -1,
            };

            res = safe_ioctl(ion_fd, ION_IOC_NEW_ALLOC, &alloc_data);
            close(ion_fd);

            if (res) return -1;
            return alloc_data.fd;
        }
    }
}

static inline uint64_t getTotalSystemMemory() {
    FILE* file = fopen("/proc/meminfo", "r");
    if (!file) return 0;
    uint64_t memTotalKB = 0;

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %ld kB", &memTotalKB);
            break;
        }
    }

    fclose(file);
    return memTotalKB * 1024;
}

static inline uint64_t getAvailableSystemMemory() {
    FILE* file = fopen("/proc/meminfo", "r");
    if (!file) return 0;
    uint64_t availMemKB = 0;

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %ld kB", &availMemKB);
            break;
        }
        else if (strncmp(line, "MemFree:", 8) == 0 && availMemKB == 0) {
            sscanf(line, "MemFree: %ld kB", &availMemKB);
        }
    }

    fclose(file);
    return availMemKB * 1024;
}

#endif