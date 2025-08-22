#include "vk_object.h"

static ArrayList cachedObjects = {0};

VkObject vkNullObject = {
    .id = VKOBJECT_NULL_ID,
    .type = VK_OBJECT_TYPE_UNKNOWN,
    .tag = NULL,
    .handle = NULL
};

static VkObject* internalCreate(VkObjectType type, uint64_t id) {
    VkObject* object = calloc(1, sizeof(VkObject));
    object->type = type;
    object->id = id;
    object->handle = VK_NULL_HANDLE;
    return object;
}

static VkObject* getCachedObject(uint64_t id) {
    for (int i = 0; i < cachedObjects.size; i++) {
        VkObject* object = cachedObjects.elements[i];
        if (object->id == id) return object;
    }

    return &vkNullObject;
}

VkObject* VkObject_create(VkObjectType type, uint64_t id) {
    if (type == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        VkObject* physicalDeviceObject = getCachedObject(id);
        if (!VKOBJECT_IS_NULL(physicalDeviceObject)) return physicalDeviceObject;

        physicalDeviceObject = internalCreate(type, id);
        ArrayList_add(&cachedObjects, physicalDeviceObject);
        return physicalDeviceObject;
    }
    else if (type == VK_OBJECT_TYPE_QUEUE) {
        VkObject* queueObject = getCachedObject(id);
        if (!VKOBJECT_IS_NULL(queueObject)) return queueObject;

        queueObject = internalCreate(type, id);
        ArrayList_add(&cachedObjects, queueObject);
        return queueObject;
    }

    return internalCreate(type, id);
}

void* VkObject_toHandle(VkObject* object) {
    if (object->handle) return object->handle;

    char* handle = calloc(2, sizeof(uint64_t));
    *(uint64_t*)(handle + sizeof(uint64_t)) = (uint64_t)object;
    object->handle = handle;
    return handle;
}

void* VkObject_fromId(uint64_t id) {
    return id != VKOBJECT_NULL_ID ? (void*)id : VK_NULL_HANDLE;
}

VkObject* VkObject_fromHandle(void* handle) {
    if (handle) {
        uint64_t ptr = *(uint64_t*)((char*)handle + sizeof(uint64_t));
        if (ptr) return (VkObject*)ptr;
    }
    return &vkNullObject;
}

void VkObject_free(VkObject* object) {
    if (!VKOBJECT_IS_NULL(object)) {
        MEMFREE(object->handle);
        MEMFREE(object);
    }
}