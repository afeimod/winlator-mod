#include <jni.h>
#include <wchar.h>
#include <android/log.h>

#include "winlator.h"
#include "string_utils.h"

#define BUFFER_SIZE 65536

struct Location {
    int offset;
    int start;
    int end;
    int mbCount;
};

static bool lineHasName(wchar_t* line) {
    if (!(*line) || *line == L'\n') return false;
    char status = 0;
    while (*line && status != 3) {
        if (status == 0 && (*line == L'"' || *line == L'@')) {
            status = *line != L'@' ? 1 : 2;
        }
        else if (status == 1 && *line == L'"') {
            status = 2;
        }
        else if (status == 2 && *line == L'=') {
            status = 3;
        }
        line++;
    }
    return status == 3;
}

JNIEXPORT jintArray JNICALL
Java_com_winlator_core_WineRegistryEditor_getKeyLocation(JNIEnv *env, jobject obj,
                                                         jstring filename, jstring key) {
    const char* filenamePtr = (*env)->GetStringUTFChars(env, filename, 0);
    wchar_t* wkey = jstringToWChars(env, key);

    FILE* file = fopen(filenamePtr, "r");
    wchar_t lineBuf[BUFFER_SIZE];
    wchar_t* lineRead;
    struct Location location = {0};
    location.start = -1;
    location.end = -1;
    int totalLen = 0;
    int emptyLines = 0;
    int lineLen;

    while ((lineRead = fgetws(lineBuf, BUFFER_SIZE, file))) {
        lineLen = wcslen(lineRead);

        if (location.start == -1) {
            if (wcstartswith(wkey, lineRead)) {
                location.offset = totalLen - 1;
                location.start = totalLen + lineLen;
            }

            location.mbCount += mbclen(lineRead);
        }
        else {
            if (wcstartswith(L"[", lineRead)) {
                location.end = totalLen - emptyLines - 1;
                if (location.end < -1) location.end = -1;
                break;
            }
            else emptyLines = lineLen == 0 ? emptyLines + 1 : 1;
        }

        totalLen += lineLen;
    }

    if (location.end == -1) location.end = totalLen - 1;

    fclose(file);
    free(wkey);
    (*env)->ReleaseStringUTFChars(env, filename, filenamePtr);

    jint locationArray[] = {location.offset, location.start, location.end, location.mbCount};
    jintArray result = (*env)->NewIntArray(env, 4);
    (*env)->SetIntArrayRegion(env, result, 0, 4, locationArray);
    return result;
}

JNIEXPORT jintArray JNICALL
Java_com_winlator_core_WineRegistryEditor_getValueLocation(JNIEnv *env, jobject obj,
                                                           jstring filename, jintArray keyLocationArr,
                                                           jstring name) {
    const char* filenamePtr = (*env)->GetStringUTFChars(env, filename, 0);
    wchar_t* wname = jstringToWChars(env, name);
    int* keyLocationPtr = (*env)->GetIntArrayElements(env, keyLocationArr, 0);
    struct Location keyLocation = {keyLocationPtr[0], keyLocationPtr[1], keyLocationPtr[2], keyLocationPtr[3]};
    (*env)->ReleaseIntArrayElements(env, keyLocationArr, keyLocationPtr, 0);

    FILE* file = fopen(filenamePtr, "r");
    wchar_t lineBuf[BUFFER_SIZE];
    wchar_t* lineRead;
    struct Location valueLocation = {0};
    valueLocation.start = -1;
    valueLocation.end = -1;
    int totalLen = keyLocation.start;
    int lineLen;

    fseek(file, keyLocation.start + keyLocation.mbCount, SEEK_SET);

    while (totalLen < keyLocation.end && (lineRead = fgetws(lineBuf, BUFFER_SIZE, file))) {
        lineLen = wcslen(lineRead);

        if (valueLocation.start == -1) {
            if (wcstartswith(wname, lineRead)) {
                valueLocation.offset = totalLen - 1;
                valueLocation.start = totalLen + wcslen(wname);
            }
        }
        else {
            if (lineHasName(lineRead)) {
                valueLocation.end = totalLen - 1;
                break;
            }
        }

        totalLen += lineLen;
    }

    if (valueLocation.end == -1) valueLocation.end = totalLen - 1;

    fclose(file);
    free(wname);
    (*env)->ReleaseStringUTFChars(env, filename, filenamePtr);

    jint locationArray[] = {valueLocation.offset, valueLocation.start, valueLocation.end, valueLocation.mbCount};
    jintArray result = (*env)->NewIntArray(env, 4);
    (*env)->SetIntArrayRegion(env, result, 0, 4, locationArray);
    return result;
}