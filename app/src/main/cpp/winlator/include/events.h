#ifndef WINLATOR_EVENTS_H
#define WINLATOR_EVENTS_H

#include <stdbool.h>
#include <poll.h>
#include <errno.h>
#include <threads.h>
#include <sys/eventfd.h>

#define EVENT_RESULT_SUCCESS 0
#define EVENT_RESULT_TIMEOUT 1
#define EVENT_RESULT_ERROR 2

#define BUSY_WAIT_MAX_ITER 500
#define BUSY_WAIT_SLEEP_US 100

static inline int createEvent(bool initialState) {
    return eventfd(initialState ? 1 : 0, EFD_CLOEXEC | EFD_NONBLOCK);
}

static inline bool setEvent(int fd) {
    uint64_t value = 1;
    ssize_t res = write(fd, &value, sizeof(uint64_t));
    return res >= 0 ? true : false;
}

static inline bool resetEvent(int fd) {
    uint64_t value;
    ssize_t res = read(fd, &value, sizeof(uint64_t));
    return res >= 0 ? true : false;
}

static inline int waitForEvents(int* fds, int numFds, bool waitAll, int timeoutMs) {
    if (timeoutMs <= 0) timeoutMs = -1;
    struct pollfd pfds[numFds];
    int i, j, res;

    bool signaled[numFds];
    for (i = 0, j = 0; i < numFds; i++) {
        if (fds[i] < 0) continue;
        pfds[j].fd = fds[i];
        pfds[j].events = POLLIN;
        pfds[j].revents = 0;
        signaled[j] = false;
        j++;
    }

    numFds = j;
    if (numFds == 0) return EVENT_RESULT_SUCCESS;

    int remaining = numFds;
    do {
        res = poll(pfds, remaining, timeoutMs);
        if (res < 0) return EVENT_RESULT_ERROR;

        bool anySignaled = false;
        for (i = 0; i < remaining; i++) {
            if ((pfds[i].revents & POLLIN)) {
                if (!waitAll || numFds == 1) return EVENT_RESULT_SUCCESS;
                remaining--;

                for (j = 0; j < numFds; j++) {
                    if (pfds[i].fd == fds[j]) {
                        signaled[j] = true;
                        anySignaled = true;
                        break;
                    }
                }
            }
        }

        if (!anySignaled && timeoutMs > 0) return EVENT_RESULT_TIMEOUT;

        for (i = 0, j = 0; i < numFds; i++) {
            if (!signaled[i]) {
                pfds[j].fd = fds[i];
                pfds[j].events = POLLIN;
                pfds[j].revents = 0;
                j++;
            }
        }
    }
    while (remaining > 0);

    return EVENT_RESULT_SUCCESS;
}

static inline void busyWait(uint32_t* iter) {
    (*iter)++;
    if (*iter < BUSY_WAIT_MAX_ITER) {
        thrd_yield();
    }
    else usleep(BUSY_WAIT_SLEEP_US);
}

#endif