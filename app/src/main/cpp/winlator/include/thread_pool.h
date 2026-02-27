#ifndef WINLATOR_THREAD_POOL_H
#define WINLATOR_THREAD_POOL_H

#include <pthread.h>

typedef struct ThreadPool_Task {
    void (*func)(void* param);
    void* param;
    void* next;
} ThreadPool_Task;

typedef struct ThreadPool {
    uint8_t numThreads;
    uint8_t numThreadsWorking;
    bool running;
    pthread_mutex_t taskMutex;
    pthread_cond_t taskCond;
    pthread_mutex_t threadCountMutex;
    pthread_cond_t threadIdleCond;
    ThreadPool_Task* firstTask;
    ThreadPool_Task* lastTask;
} ThreadPool;

static inline ThreadPool_Task* ThreadPool_getTask(ThreadPool* threadPool) {
    if (!threadPool) return NULL;
    ThreadPool_Task* task = threadPool->firstTask;
    if (!task) return NULL;

    if (!task->next) {
        threadPool->firstTask = NULL;
        threadPool->lastTask  = NULL;
    }
    else threadPool->firstTask = task->next;
    return task;
}

static inline void* threadPoolWorker(void* param) {
    ThreadPool* threadPool = param;
    ThreadPool_Task* task;

    while (threadPool->running) {
        pthread_mutex_lock(&threadPool->taskMutex);

        while (!threadPool->firstTask && threadPool->running) {
            pthread_cond_wait(&threadPool->taskCond, &threadPool->taskMutex);
        }

        pthread_mutex_lock(&threadPool->threadCountMutex);
        threadPool->numThreadsWorking++;
        task = ThreadPool_getTask(threadPool);
        pthread_mutex_unlock(&threadPool->threadCountMutex);

        pthread_mutex_unlock(&threadPool->taskMutex);

        if (task) {
            task->func(task->param);
            free(task);
        }

        pthread_mutex_lock(&threadPool->threadCountMutex);
        threadPool->numThreadsWorking--;
        if (threadPool->numThreadsWorking == 0) pthread_cond_signal(&threadPool->threadIdleCond);
        pthread_mutex_unlock(&threadPool->threadCountMutex);
    }

    pthread_mutex_lock(&threadPool->threadCountMutex);
    threadPool->numThreads--;
    pthread_cond_signal(&threadPool->threadIdleCond);
    pthread_mutex_unlock(&threadPool->threadCountMutex);
    return NULL;
}

static inline ThreadPool* ThreadPool_init(int numThreads) {
    ThreadPool* threadPool = calloc(1, sizeof(ThreadPool));
    threadPool->numThreads = numThreads;
    threadPool->numThreadsWorking = 0;
    threadPool->running = true;

    pthread_mutex_init(&threadPool->taskMutex, NULL);
    pthread_mutex_init(&threadPool->threadCountMutex, NULL);
    pthread_cond_init(&threadPool->taskCond, NULL);
    pthread_cond_init(&threadPool->threadIdleCond, NULL);

    for (int i = 0; i < numThreads; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, threadPoolWorker, threadPool);
        pthread_detach(thread);
    }

    return threadPool;
}

static inline void ThreadPool_run(ThreadPool* threadPool, void (*func)(void*), void* param) {
    ThreadPool_Task* task = malloc(sizeof(ThreadPool_Task));
    task->func = func;
    task->param = param;
    task->next = NULL;

    pthread_mutex_lock(&threadPool->taskMutex);

    if (!threadPool->firstTask) {
        threadPool->firstTask = task;
        threadPool->lastTask = threadPool->firstTask;
    }
    else {
        threadPool->lastTask->next = task;
        threadPool->lastTask = task;
    }

    pthread_cond_signal(&threadPool->taskCond);
    pthread_mutex_unlock(&threadPool->taskMutex);
}

static inline void ThreadPool_wait(ThreadPool* threadPool) {
    pthread_mutex_lock(&threadPool->threadCountMutex);
    while ((!threadPool->running && threadPool->numThreads > 0) || threadPool->firstTask || threadPool->numThreadsWorking > 0) {
        pthread_cond_wait(&threadPool->threadIdleCond, &threadPool->threadCountMutex);
    }
    pthread_mutex_unlock(&threadPool->threadCountMutex);
}

static inline void ThreadPool_destroy(ThreadPool* threadPool) {
    if (!threadPool) return;
    pthread_mutex_lock(&threadPool->taskMutex);

    ThreadPool_Task* curr = threadPool->firstTask;
    ThreadPool_Task* next;
    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }

    threadPool->firstTask = NULL;
    threadPool->lastTask = NULL;
    threadPool->running = false;

    pthread_cond_broadcast(&threadPool->taskCond);
    pthread_mutex_unlock(&threadPool->taskMutex);

    ThreadPool_wait(threadPool);

    pthread_mutex_destroy(&threadPool->taskMutex);
    pthread_cond_destroy(&threadPool->taskCond);
    pthread_cond_destroy(&threadPool->threadIdleCond);

    free(threadPool);
}

#endif