#ifndef VORTEK_TIMELINE_SEMAPHORE_H
#define VORTEK_TIMELINE_SEMAPHORE_H

#include "vortek.h"

extern void TimelineSemaphore_asyncWait(int clientFd, ThreadPool* threadPool, char* inputBuffer, int inputBufferSize);

#endif