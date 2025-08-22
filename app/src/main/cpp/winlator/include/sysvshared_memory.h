#ifndef WINLATOR_SYSVSHARED_MEMORY_H
#define WINLATOR_SYSVSHARED_MEMORY_H

#include <sys/mman.h>

extern int ashmemCreateRegion(const char* name, int64_t size);
extern int createMemoryFd(const char* name, int64_t size);

#endif