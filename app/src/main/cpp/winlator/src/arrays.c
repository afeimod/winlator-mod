#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>

#include "arrays.h"
#include "string_utils.h"

static int intCompare(const void * a, const void * b) {
    return (*(int*)a - *(int*)b);
}

static int hashCode(const char* key) {
    int hash = 0;
    for (const char* p = key; *p; p++) hash = 31 * hash + *p;
    return hash;
}

void IntArray_add(IntArray* intArray, int value) {
    ENSURE_ARRAY_CAPACITY(intArray->size + 1, intArray->capacity, intArray->values, sizeof(int));
    intArray->values[intArray->size++] = value;
}

void IntArray_addAll(IntArray* intArray, int valueCount, ...) {
    va_list valist;
    va_start(valist, valueCount);

    ENSURE_ARRAY_CAPACITY(intArray->size + valueCount, intArray->capacity, intArray->values, sizeof(int));
    for (int i = 0; i < valueCount; i++) intArray->values[intArray->size++] = va_arg(valist, int);

    va_end(valist);
}

void IntArray_remove(IntArray* intArray, int offset, int count) {
    size_t newSize;
    int* newValues = memdel(intArray->values, intArray->size * sizeof(int), offset * sizeof(int), count * sizeof(int), &newSize);
    if (newValues) {
        int* oldValues = intArray->values;
        intArray->size = newSize / sizeof(int);
        intArray->values = newValues;
        free(oldValues);
    }
    else if (intArray->size <= 1) {
        IntArray_clear(intArray);
    }
}

int IntArray_removeAt(IntArray* intArray, int index) {
    if (index < 0 || index >= intArray->size) return -1;
    int oldValue = intArray->values[index];
    IntArray_remove(intArray, index, 1);
    return oldValue;
}

void IntArray_clear(IntArray* intArray) {
    if (intArray && intArray->values) {
        free(intArray->values);
        intArray->values = NULL;
        intArray->size = 0;
        intArray->capacity = 0;
    }
}

void IntArray_sort(IntArray* intArray) {
    if (intArray) qsort(intArray->values, intArray->size, sizeof(int), intCompare);
}

int ArrayList_indexOf(ArrayList* arrayList, void* element) {
    for (int i = 0; i < arrayList->size; i++) {
        if (arrayList->elements[i] == element) return i;
    }
    return -1;
}

void ArrayList_add(ArrayList* arrayList, void* element) {
    if (!element) return;
    ENSURE_ARRAY_CAPACITY(arrayList->size + 1, arrayList->capacity, arrayList->elements, sizeof(void*));
    arrayList->elements[arrayList->size++] = element;
}

void ArrayList_fill(ArrayList* arrayList, int size, void* element) {
    if (!element) return;
    ENSURE_ARRAY_CAPACITY(arrayList->size + size, arrayList->capacity, arrayList->elements, sizeof(void*));
    for (int i = 0; i < size; i++) arrayList->elements[arrayList->size++] = element;
}

void* ArrayList_removeAt(ArrayList* arrayList, int index) {
    if (index < 0 || index >= arrayList->size) return NULL;

    void* element = arrayList->elements[index];
    int movedCount = arrayList->size - index - 1;
    if (movedCount > 0) memmove(arrayList->elements + index, arrayList->elements + index + 1, movedCount * sizeof(void*));
    arrayList->elements[--arrayList->size] = NULL;
    return element;
}

void ArrayList_remove(ArrayList* arrayList, void* element) {
    int index = ArrayList_indexOf(arrayList, element);
    if (index != -1) ArrayList_removeAt(arrayList, index);
}

void ArrayList_free(ArrayList* arrayList) {
    if (!arrayList) return;

    if (arrayList->elements) {
        for (int i = 0; i < arrayList->size; i++) {
            if (arrayList->elements[i]) {
                free(arrayList->elements[i]);
                arrayList->elements[i] = NULL;
            }
        }

        free(arrayList->elements);
        arrayList->elements = NULL;
    }
}

ArrayList* ArrayList_fromStrings(const char** strings, int size) {
    ArrayList* arrayList = calloc(1, sizeof(ArrayList));
    for (int i = 0; i < size; i++) ArrayList_add(arrayList, strdup(strings[i]));
    return arrayList;
}

static int ArrayMap_binarySearch(ArrayMap* arrayMap, const char* key) {
    int lo = 0;
    int hi = arrayMap->size - 1;
    int hash = hashCode(key);

    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int midHash = arrayMap->entries[mid].key ? hashCode(arrayMap->entries[mid].key) : 0;

        if (midHash < hash) {
            lo = mid + 1;
        }
        else if (midHash > hash) {
            hi = mid - 1;
        }
        else return mid;
    }
    return ~lo;
}

int ArrayMap_indexOfKey(ArrayMap* arrayMap, const char* key) {
    if (arrayMap->size == 0) return -1;
    int index = ArrayMap_binarySearch(arrayMap, key);
    if (index < 0) return index;
    if (strcmp(key, arrayMap->entries[index].key) == 0) return index;

    int end;
    int hash = hashCode(key);
    for (end = index + 1; end < arrayMap->size && hashCode(arrayMap->entries[end].key) == hash; end++) {
        if (strcmp(key, arrayMap->entries[end].key) == 0) return end;
    }

    for (int i = index - 1; i >= 0 && hashCode(arrayMap->entries[i].key) == hash; i--) {
        if (strcmp(key, arrayMap->entries[i].key) == 0) return i;
    }
    return ~end;
}

int ArrayMap_indexOfValue(ArrayMap* arrayMap, void* value) {
    for (int i = 0; i < arrayMap->size; i++) {
        if (arrayMap->entries[i].value == value) return i;
    }
    return -1;
}

void ArrayMap_put(ArrayMap* arrayMap, const char* key, void* value) {
    int index = ArrayMap_indexOfKey(arrayMap, key);
    if (index >= 0) {
        arrayMap->entries[index].value = value;
        return;
    }

    index = ~index;
    ENSURE_ARRAY_CAPACITY(arrayMap->size + 1, arrayMap->capacity, arrayMap->entries, sizeof(ArrayMap_Entry));

    if (index < arrayMap->size) {
        memmove(arrayMap->entries + index + 1, arrayMap->entries + index, (arrayMap->size - index) * sizeof(ArrayMap_Entry));
    }

    arrayMap->entries[index].key = key;
    arrayMap->entries[index].value = value;
    arrayMap->size++;
}

void* ArrayMap_get(ArrayMap* arrayMap, const char* key) {
    int index = ArrayMap_indexOfKey(arrayMap, key);
    return index >= 0 ? arrayMap->entries[index].value : NULL;
}

void* ArrayMap_removeAt(ArrayMap* arrayMap, int index) {
    if (index >= arrayMap->size) return NULL;
    void* oldValue = arrayMap->entries[index].value;
    memmove(arrayMap->entries + index, arrayMap->entries + index + 1, (arrayMap->size - (index + 1)) * sizeof(ArrayMap_Entry));
    arrayMap->size--;
    arrayMap->entries[arrayMap->size].key = NULL;
    arrayMap->entries[arrayMap->size].value = NULL;
    return oldValue;
}

void* ArrayMap_remove(ArrayMap* arrayMap, const char* key) {
    int index = ArrayMap_indexOfKey(arrayMap, key);
    return index >= 0 ? ArrayMap_removeAt(arrayMap, index) : NULL;
}

void ArrayMap_free(ArrayMap* arrayMap, bool freeKeys) {
    if (arrayMap->entries) {
        if (freeKeys) {
            for (int i = 0; i < arrayMap->capacity; i++) {
                if (arrayMap->entries[i].key) free(arrayMap->entries[i].key);
            }
        }
        free(arrayMap->entries);
        arrayMap->entries = NULL;
    }

    arrayMap->size = 0;
    arrayMap->capacity = 0;
}

static int SparseArray_binarySearch(SparseArray* sparseArray, int key) {
    int lo = 0;
    int hi = sparseArray->size - 1;

    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int midVal = sparseArray->entries[mid].key;

        if (midVal < key) {
            lo = mid + 1;
        }
        else if (midVal > key) {
            hi = mid - 1;
        }
        else return mid;
    }
    return ~lo;
}

int SparseArray_indexOfKey(SparseArray* sparseArray, int key) {
    if (!sparseArray->entries) return -1;
    return SparseArray_binarySearch(sparseArray, key);
}

int SparseArray_indexOfValue(SparseArray* sparseArray, void* value) {
    for (int i = 0; i < sparseArray->size; i++) {
        if (sparseArray->entries[i].value == value) return i;
    }
    return -1;
}

void SparseArray_put(SparseArray* sparseArray, int key, void* value) {
    int index = SparseArray_indexOfKey(sparseArray, key);
    if (index >= 0) {
        sparseArray->entries[index].value = value;
        return;
    }

    index = ~index;
    ENSURE_ARRAY_CAPACITY(sparseArray->size + 1, sparseArray->capacity, sparseArray->entries, sizeof(SparseArray_Entry));

    if (index < sparseArray->size) {
        memmove(sparseArray->entries + index + 1, sparseArray->entries + index, (sparseArray->size - index) * sizeof(SparseArray_Entry));
    }

    sparseArray->entries[index].key = key;
    sparseArray->entries[index].value = value;
    sparseArray->size++;
}

void* SparseArray_get(SparseArray* sparseArray, int key) {
    int index = SparseArray_indexOfKey(sparseArray, key);
    return index >= 0 ? sparseArray->entries[index].value : NULL;
}

void* SparseArray_removeAt(SparseArray* sparseArray, int index) {
    if (index >= sparseArray->size) return NULL;
    void* oldValue = sparseArray->entries[index].value;
    memmove(sparseArray->entries + index, sparseArray->entries + index + 1, (sparseArray->size - (index + 1)) * sizeof(SparseArray_Entry));
    sparseArray->size--;
    sparseArray->entries[sparseArray->size].key = 0;
    sparseArray->entries[sparseArray->size].value = NULL;
    return oldValue;
}

void* SparseArray_remove(SparseArray* sparseArray, int key) {
    int index = SparseArray_indexOfKey(sparseArray, key);
    return index >= 0 ? SparseArray_removeAt(sparseArray, index) : NULL;
}

static void ArrayDeque_doubleCapacity(ArrayDeque* arrayDeque) {
    int head = arrayDeque->head;
    int size = arrayDeque->size;
    int diff = size - head;
    int newCapacity = size << 1;
    if (newCapacity < 0) return;

    void** newElements = malloc(newCapacity * sizeof(void*));
    memcpy(newElements, arrayDeque->elements + head, diff * sizeof(void*));
    memcpy(newElements + diff, arrayDeque->elements, head * sizeof(void*));
    free(arrayDeque->elements);

    arrayDeque->elements = newElements;
    arrayDeque->size = newCapacity;
    arrayDeque->head = 0;
    arrayDeque->tail = size;
}

void ArrayDeque_init(ArrayDeque* arrayDeque, int initialCapacity) {
    if (initialCapacity < 8) initialCapacity = 8;

    initialCapacity |= (initialCapacity >>  1);
    initialCapacity |= (initialCapacity >>  2);
    initialCapacity |= (initialCapacity >>  4);
    initialCapacity |= (initialCapacity >>  8);
    initialCapacity |= (initialCapacity >> 16);
    initialCapacity++;

    if (initialCapacity < 0) initialCapacity >>= 1;

    arrayDeque->head = 0;
    arrayDeque->tail = 0;
    arrayDeque->size = initialCapacity;
    arrayDeque->elements = malloc(initialCapacity * sizeof(void*));
}

bool ArrayDeque_isEmpty(ArrayDeque* arrayDeque) {
    return arrayDeque && arrayDeque->head == arrayDeque->tail;
}

void ArrayDeque_addFirst(ArrayDeque* arrayDeque, void* element) {
    arrayDeque->head = (arrayDeque->head - 1) & (arrayDeque->size - 1);
    arrayDeque->elements[arrayDeque->head] = element;
    if (arrayDeque->head == arrayDeque->tail) ArrayDeque_doubleCapacity(arrayDeque);
}

void ArrayDeque_addLast(ArrayDeque* arrayDeque, void* element) {
    arrayDeque->elements[arrayDeque->tail] = element;
    arrayDeque->tail = (arrayDeque->tail + 1) & (arrayDeque->size - 1);
    if (arrayDeque->tail == arrayDeque->head) ArrayDeque_doubleCapacity(arrayDeque);
}

void* ArrayDeque_removeFirst(ArrayDeque* arrayDeque) {
    int head = arrayDeque->head;
    void* result = arrayDeque->elements[head];
    if (result) {
        arrayDeque->elements[head] = NULL;
        arrayDeque->head = (head + 1) & (arrayDeque->size - 1);
    }
    return result;
}

void* ArrayDeque_removeLast(ArrayDeque* arrayDeque) {
    int tail = (arrayDeque->tail - 1) & (arrayDeque->size - 1);
    void* result = arrayDeque->elements[tail];
    if (result) {
        arrayDeque->elements[tail] = NULL;
        arrayDeque->tail = tail;
    }
    return result;
}

void ArrayDeque_free(ArrayDeque* arrayDeque) {
    if (!arrayDeque) return;
    for (int i = 0; i < arrayDeque->size; i++) {
        if (arrayDeque->elements[i]) {
            free(arrayDeque->elements[i]);
            arrayDeque->elements[i] = NULL;
        }
    }
    free(arrayDeque);
}