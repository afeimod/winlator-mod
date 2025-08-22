#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array)[0])
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define PACK16(a,b) (a << 16) | (b & 0xffff)

#define APP_CACHE_DIR "/data/data/com.winlator/cache"
#define LIBVULKAN_PATH "/system/lib64/libvulkan.so"

#define CLOSEFD(x) \
    do { \
        if (x > 0) { \
            close(x); \
            x = -1; \
        } \
    } \
    while(0)

#define MEMFREE(x) \
    do { \
        if (x != NULL) { \
            free(x); \
            x = NULL; \
        } \
    } \
    while(0)

#ifndef WINLATOR_H
#define WINLATOR_H

#ifdef __ANDROID__
#include <android/log.h>
#define println(...) __android_log_print(ANDROID_LOG_DEBUG, "System.out", __VA_ARGS__)
#else

#define println(fmt, ...) \
    do { \
        char fmtBuf[256] = {0}; \
        sprintf(fmtBuf, "%s\n", fmt); \
        fprintf(stderr, fmtBuf __VA_OPT__(,) __VA_ARGS__); \
    } \
    while (0)
#endif

#endif