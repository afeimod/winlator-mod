#ifndef WINLATOR_ARRAYS_H
#define WINLATOR_ARRAYS_H

#include <malloc.h>
#include <stdbool.h>

typedef struct ArrayBuffer {
    int size;
    int position;
    int capacity;
    char* buffer;
} ArrayBuffer;

extern void ArrayBuffer_put(ArrayBuffer* arrayBuffer, char value);
extern void ArrayBuffer_putShort(ArrayBuffer* arrayBuffer, short value);
extern void ArrayBuffer_putInt(ArrayBuffer* arrayBuffer, int value);
extern void ArrayBuffer_putLong(ArrayBuffer* arrayBuffer, long value);
extern void ArrayBuffer_putFloat(ArrayBuffer* arrayBuffer, float value);
extern float* ArrayBuffer_putFloat2(ArrayBuffer* arrayBuffer, float x, float y);
extern float* ArrayBuffer_putFloat3(ArrayBuffer* arrayBuffer, float x, float y, float z);
extern float* ArrayBuffer_putFloat4(ArrayBuffer* arrayBuffer, float x, float y, float z, float w);
extern void ArrayBuffer_putDouble(ArrayBuffer* arrayBuffer, double value);
extern void ArrayBuffer_putBytes(ArrayBuffer* arrayBuffer, const void* bytes, int size);
extern char ArrayBuffer_get(ArrayBuffer* arrayBuffer);
extern short ArrayBuffer_getShort(ArrayBuffer* arrayBuffer);
extern int ArrayBuffer_getInt(ArrayBuffer* arrayBuffer);
extern long ArrayBuffer_getLong(ArrayBuffer* arrayBuffer);
extern float ArrayBuffer_getFloat(ArrayBuffer* arrayBuffer);
extern double ArrayBuffer_getDouble(ArrayBuffer* arrayBuffer);
extern void* ArrayBuffer_getBytes(ArrayBuffer* arrayBuffer, int size);
extern void ArrayBuffer_skip(ArrayBuffer* arrayBuffer, int length);
extern int ArrayBuffer_available(ArrayBuffer* arrayBuffer);
extern void ArrayBuffer_rewind(ArrayBuffer* arrayBuffer);
extern void ArrayBuffer_free(ArrayBuffer* arrayBuffer);

typedef struct IntArray {
    int size;
    int capacity;
    int* values;
} IntArray;

extern void IntArray_add(IntArray* intArray, int value);
extern void IntArray_addAt(IntArray* intArray, int index, int value);
extern void IntArray_addAll(IntArray* intArray, int valueCount, ...);
extern void IntArray_remove(IntArray* intArray, int offset, int count);
extern int IntArray_removeAt(IntArray* intArray, int index);
extern void IntArray_clear(IntArray* intArray);
extern void IntArray_sort(IntArray* intArray);

typedef struct ArrayList {
    int size;
    int capacity;
    void** elements;
} ArrayList;

extern int ArrayList_indexOf(ArrayList* arrayList, void* element);
extern void ArrayList_add(ArrayList* arrayList, void* element);
extern void ArrayList_addAt(ArrayList* arrayList, int index, void* element);
extern void ArrayList_fill(ArrayList* arrayList, int size, void* element);
extern void* ArrayList_removeAt(ArrayList* arrayList, int index);
extern void ArrayList_remove(ArrayList* arrayList, void* element);
extern void ArrayList_free(ArrayList* arrayList);
extern ArrayList* ArrayList_fromStrings(const char** strings, int size);
extern int ArrayList_containsString(ArrayList* arrayList, const char* string);

typedef struct ArrayMap_Entry {
    char* key;
    void* value;
} ArrayMap_Entry;

typedef struct ArrayMap {
    int size;
    int capacity;
    ArrayMap_Entry* entries;
} ArrayMap;

extern int ArrayMap_indexOfKey(ArrayMap* arrayMap, const char* key);
extern int ArrayMap_indexOfValue(ArrayMap* arrayMap, void* value);
extern void ArrayMap_put(ArrayMap* arrayMap, const char* key, void* value);
extern void* ArrayMap_get(ArrayMap* arrayMap, const char* key);
extern void* ArrayMap_removeAt(ArrayMap* arrayMap, int index);
extern void* ArrayMap_remove(ArrayMap* arrayMap, const char* key);
extern void ArrayMap_free(ArrayMap* arrayMap, bool freeKeys);

typedef struct SparseArray_Entry {
    int key;
    void* value;
} SparseArray_Entry;

typedef struct SparseArray {
    int size;
    int capacity;
    SparseArray_Entry* entries;
} SparseArray;

extern int SparseArray_indexOfKey(SparseArray* sparseArray, int key);
extern int SparseArray_indexOfValue(SparseArray* sparseArray, void* value);
extern void SparseArray_put(SparseArray* sparseArray, int key, void* value);
extern void* SparseArray_get(SparseArray* sparseArray, int key);
extern void* SparseArray_removeAt(SparseArray* sparseArray, int index);
extern void* SparseArray_remove(SparseArray* sparseArray, int key);

typedef struct ArrayDeque {
    int head;
    int tail;
    int size;
    void** elements;
} ArrayDeque;

extern void ArrayDeque_init(ArrayDeque* arrayDeque, int initialCapacity);
extern bool ArrayDeque_isEmpty(ArrayDeque* arrayDeque);
extern void ArrayDeque_addFirst(ArrayDeque* arrayDeque, void* element);
extern void ArrayDeque_addLast(ArrayDeque* arrayDeque, void* element);
extern void* ArrayDeque_removeFirst(ArrayDeque* arrayDeque);
extern void* ArrayDeque_removeLast(ArrayDeque* arrayDeque);
extern void ArrayDeque_free(ArrayDeque* arrayDeque);

#define DEFAULT_ARRAY_CAPACITY 4
#define ENSURE_ARRAY_CAPACITY(targetSize, capacity, elements, elementSize) \
    do { \
        if (targetSize > capacity) { \
            int newCapacity = elementSize > 1 ? (capacity < DEFAULT_ARRAY_CAPACITY ? DEFAULT_ARRAY_CAPACITY : capacity + (capacity >> 1)) : 0; \
            if (newCapacity < targetSize) newCapacity = targetSize; \
            elements = realloc(elements, newCapacity * elementSize); \
            capacity = newCapacity; \
        } \
    } \
    while (0)

#endif