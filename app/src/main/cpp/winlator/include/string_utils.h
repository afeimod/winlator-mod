#ifndef WINLATOR_STRING_UTILS_H
#define WINLATOR_STRING_UTILS_H

#include "arrays.h"

#ifdef __ANDROID__
#include <jni.h>

static inline ArrayList* jstringArrayToCharArray(JNIEnv* env, jobjectArray stringArray) {
    if (!stringArray) return NULL;
    int stringCount = (*env)->GetArrayLength(env, stringArray);
    ArrayList* arrayList = calloc(1, sizeof(ArrayList));

    for (int i = 0; i < stringCount; i++) {
        jstring string = (jstring)((*env)->GetObjectArrayElement(env, stringArray, i));
        const char* rawString = (*env)->GetStringUTFChars(env, string, 0);
        ArrayList_add(arrayList, strdup(rawString));
        (*env)->ReleaseStringUTFChars(env, string, rawString);
    }

    return arrayList;
}

static inline wchar_t* jstringToWChars(JNIEnv* env, jstring string) {
    const jchar* stringPtr = (*env)->GetStringChars(env, string, 0);
    size_t len = (*env)->GetStringLength(env, string);

    wchar_t* result = calloc(len + 1, sizeof(wchar_t));
    int i = 0;
    while (len > 0) {
        result[i] = stringPtr[i];
        len--;
        i++;
    }

    (*env)->ReleaseStringChars(env, string, stringPtr);
    return result;
}
#endif

static inline bool wstrstarts(wchar_t* prefix, wchar_t* string) {
    while (*prefix) if (*prefix++ != *string++) return false;
    return true;
}

static inline int mbclen(wchar_t* string) {
    int count = 0;
    while (*string) if (*string++ > 0x7f) count++;
    return count;
}

static inline void* memmerge(void* src, size_t srcSize, void* dst, size_t dstSize, size_t dstOffset, size_t* newSize) {
    if (dstOffset > dstSize) dstOffset = dstSize;
    void* res = malloc(srcSize + dstSize);
    memcpy(res, dst, dstOffset);
    memcpy(res + dstOffset, src, srcSize);
    memcpy(res + dstOffset + srcSize, dst + dstOffset, dstSize - dstOffset);
    if (newSize) *newSize = srcSize + dstSize;
    return res;
}

static inline void* memdel(void* src, size_t size, size_t offset, size_t delSize, size_t* newSize) {
    if (newSize) *newSize = 0;
    int movedCount = size - offset - delSize;
    if (movedCount <= 0) return NULL;

    void* dst = malloc(size - delSize);
    memcpy(dst, src, offset);
    memcpy(dst + offset, src + offset + delSize, movedCount);
    if (newSize) *newSize = size - delSize;
    return dst;
}

static inline void* memdup(const void* src, size_t size) {
    void* dst = malloc(size);
    if (dst != NULL) memcpy(dst, src, size);
    return dst;
}

// MurmurHash2, 64-bit version, by Austin Appleby
static inline uint64_t murmurHash64(const char* key, size_t len, uint64_t seed) {
    const uint64_t kMul = 0xc6a4a7935bd1e995UL;
    const int kShift = 47;
    uint64_t hash = seed ^ (len * kMul);
    const uint64_t* ptr = (const uint64_t*)key;
    const uint64_t* end = ptr + (len / 8);

    while (ptr != end) {
        uint64_t k = *ptr++;

        k *= kMul;
        k ^= k >> kShift;
        k *= kMul;

        hash ^= k;
        hash *= kMul;
    }

    const u_char* data = (const u_char*)ptr;
    switch (len & 7) {
        case 7: hash ^= ((uint64_t)data[6]) << 48;
        case 6: hash ^= ((uint64_t)data[5]) << 40;
        case 5: hash ^= ((uint64_t)data[4]) << 32;
        case 4: hash ^= ((uint64_t)data[3]) << 24;
        case 3: hash ^= ((uint64_t)data[2]) << 16;
        case 2: hash ^= ((uint64_t)data[1]) << 8;
        case 1: hash ^= ((uint64_t)data[0]);
                hash *= kMul;
    };

    hash ^= hash >> kShift;
    hash *= kMul;
    hash ^= hash >> kShift;
    return hash;
}

#endif