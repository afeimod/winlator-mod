#ifndef VK_OBJECT_H
#define VK_OBJECT_H

#include "vortek.h"

#define VKOBJECT_NULL_ID 0
#define VKOBJECT_IS_NULL(obj) (obj == NULL || obj->id == VKOBJECT_NULL_ID)

typedef struct VkObject {
    uint64_t id;
    VkObjectType type;
    void* tag;
    void* handle;
} VkObject;

extern VkObject* VkObject_create(VkObjectType type, uint64_t id);
extern void VkObject_free(VkObject* object);
extern void* VkObject_toHandle(VkObject* object);
extern void* VkObject_fromId(uint64_t id);
extern VkObject* VkObject_fromHandle(void* handle);
extern VkObject vkNullObject;

#endif