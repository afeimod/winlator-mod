#ifndef WINLATOR_FILE_UTILS_H
#define WINLATOR_FILE_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "time_utils.h"

typedef struct FindFileInfo {
    char path[PATH_MAX];
    uint64_t lastModified;
    size_t size;
} FindFileInfo;

static inline void joinPaths(char* result, const char* path1, const char* path2) {
    size_t length = strlen(path1);
    strcpy(result, path1);

    if (length > 0 && result[length-1] != '/' && path2[0] != '/') {
        strcat(result + length, "/");
        strcat(result + length, path2);
    }
    else if (length > 0 && result[length-1] == '/' && path2[0] == '/') {
        strcat(result + length, path2 + 1);
    }
    else strcat(result + length, path2);
}

static inline void createDirectory(char* path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) mkdir(path, 0771);
}

static inline bool filePutContents(char* filename, void* data, size_t size) {
    FILE* file = fopen(filename, "wb");
    if (!file) return 0;

    size_t bytesWritten = fwrite(data, 1, size, file);
    fclose(file);
    return bytesWritten == size;
}

static inline void* fileGetContents(char* filename, void* outData, size_t* outSize) {
    FILE* file = fopen(filename, "rb");
    if (!file) return NULL;

    if (outData) {
        size_t bytesRead = fread(outData, 1, *outSize, file);
        fclose(file);
        return bytesRead == *outSize ? outData : NULL;
    }
    else {
        fseek(file, 0, SEEK_END);
        uint64_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* buffer = malloc(fileSize + 1);
        fread(buffer, 1, fileSize, file);
        buffer[fileSize] = '\0';

        fclose(file);
        if (outSize) *outSize = fileSize;
        return buffer;
    }
}

static inline bool findFirstFile(char* path, FindFileInfo* fileInfo) {
    DIR* dir = opendir(path);
    fileInfo->path[0] = '\0';
    fileInfo->lastModified = 0;
    if (!dir) return false;

    char filename[PATH_MAX] = {0};
    char lastName[NAME_MAX] = {0};
    uint64_t lastModified = UINT64_MAX;
    size_t fileSize = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        struct stat st = {0};
        joinPaths(filename, path, entry->d_name);
        if (stat(filename, &st) == 0) {
            uint64_t ms = timespecToMillis(&st.st_mtim);
            if (ms < lastModified) {
                strcpy(lastName, entry->d_name);
                lastModified = ms;
                fileSize = st.st_size;
            }
        }
    }
    closedir(dir);

    if (lastModified < UINT64_MAX) {
        joinPaths(fileInfo->path, path, lastName);
        fileInfo->lastModified = lastModified;
        fileInfo->size = fileSize;
        return true;
    }
    else return false;
}

#endif