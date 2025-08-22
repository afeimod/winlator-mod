#include <string.h>

#include "vulkan_helper.h"

#define VK_MAKE_VERSION_STR(s, v) sprintf(s, "%d.%d.%d", VK_VERSION_MAJOR(v), VK_VERSION_MINOR(v), VK_VERSION_PATCH(v))

static VkMemoryType* memoryTypes = {0};
static int memoryTypeCount = 0;

#if ENABLE_VALIDATION_LAYER
static VkBool32 debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type, uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData) {
    println("Vulkan (%s): %s", pLayerPrefix, pMessage);
    return VK_FALSE;
}
#endif

static bool isExternalMemoryHandleTypeSupported(VkPhysicalDevice physicalDevice, VkExternalMemoryHandleTypeFlagBits handleType) {
    VkPhysicalDeviceExternalBufferInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO;
    bufferInfo.handleType = handleType;

    VkExternalBufferProperties bufferProperties = {0};
    bufferProperties.sType = VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES;
    vulkanWrapper.vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, &bufferInfo, &bufferProperties);
    VkExternalMemoryProperties* memoryProperties = &bufferProperties.externalMemoryProperties;

    return (memoryProperties->externalMemoryFeatures & (VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT | VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT)) != 0 &&
           (memoryProperties->compatibleHandleTypes & handleType) != 0;
}

static void setupExposedDeviceExtensions(VkContext* context, const char* engineName) {
    ArrayList_free(context->disabledDeviceExtensions);
    context->disabledDeviceExtensions = NULL;

    if (engineName && strstr(engineName, "zink")) {
        const char* disabledDeviceExtensions[] = {"VK_EXT_extended_dynamic_state", "VK_EXT_color_write_enable", "VK_KHR_push_descriptor"};
        ArrayList_free(context->exposedDeviceExtensions);
        context->disabledDeviceExtensions = ArrayList_fromStrings(disabledDeviceExtensions, ARRAY_SIZE(disabledDeviceExtensions));
        context->exposedDeviceExtensions = ArrayList_fromStrings(globalExposedDeviceExtensions, ARRAY_SIZE(globalExposedDeviceExtensions));
    }
    else {
        const char* disabledDeviceExtensions[] = {"VK_KHR_shader_float_controls", "VK_EXT_hdr_metadata", "VK_EXT_swapchain_maintenance1"};
        context->disabledDeviceExtensions = ArrayList_fromStrings(disabledDeviceExtensions, ARRAY_SIZE(disabledDeviceExtensions));
    }
}

void initVulkanInstance(VkContext* context, VkInstance instance, const VkApplicationInfo* applicationInfo) {
    VkResult result;

    loadVulkanInstanceFuncs(&vulkanWrapper, instance);

    uint32_t physicalDeviceCount = 1;
    result = vulkanWrapper.vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    if (result != VK_SUCCESS) return;

    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    result = vulkanWrapper.vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);
    if (result != VK_SUCCESS || physicalDeviceCount == 0) return;

    if (memoryTypeCount == 0) {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vulkanWrapper.vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &memoryProperties);

        memoryTypes = malloc(memoryProperties.memoryTypeCount * sizeof(VkMemoryType));
        memcpy(memoryTypes, memoryProperties.memoryTypes, memoryProperties.memoryTypeCount * sizeof(VkMemoryType));
        memoryTypeCount = memoryProperties.memoryTypeCount;
    }

    context->hasExternalMemoryFd = isExternalMemoryHandleTypeSupported(physicalDevices[0], VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT);
    context->hasExternalMemoryDMABuf = isExternalMemoryHandleTypeSupported(physicalDevices[0], VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);

    setupExposedDeviceExtensions(context, applicationInfo ? applicationInfo->pEngineName : NULL);

#if ENABLE_VALIDATION_LAYER
    VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {0};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    debugCreateInfo.pfnCallback = debugReportCallback;

    vulkanWrapper.vkCreateDebugReportCallback(instance, &debugCreateInfo, NULL, &context->debugReportCallback);
#endif
}

void initVulkanDevice(VkContext* context, VkPhysicalDevice physicalDevice, VkDevice device) {
    loadVulkanDeviceFuncs(&vulkanWrapper, device);

    uint32_t queueFamilyCount = 0;
    vulkanWrapper.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vulkanWrapper.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);

    context->graphicsQueueIndex = 0;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            context->graphicsQueueIndex = i;
            break;
        }
    }

    VkPhysicalDeviceFeatures supportedFeatures = {0};
    vulkanWrapper.vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

    if (!context->textureDecoder) context->textureDecoder = TextureDecoder_create(&supportedFeatures, context->imageCacheSize, context->threadPool);
    if (!context->shaderInspector) context->shaderInspector = ShaderInspector_create(physicalDevice, &supportedFeatures);
    if (context->textureDecoder) context->textureDecoder->threadPool = context->threadPool;
}

uint32_t getMemoryTypeIndex(uint32_t typeBits, VkFlags properties) {
    for (uint32_t i = 0; i < memoryTypeCount; i++) {
        if ((typeBits & 1) && (memoryTypes[i].propertyFlags & properties)) return i;
        typeBits >>= 1;
    }
    return 0;
}

uint32_t getMemoryPropertyFlags(uint32_t memoryTypeIndex) {
    if (memoryTypeCount == 0 || memoryTypeIndex >= memoryTypeCount) return 0;
    return memoryTypes[memoryTypeIndex].propertyFlags;
}

bool isHostVisibleMemory(uint32_t memoryTypeIndex) {
    if (memoryTypeCount == 0 || memoryTypeIndex >= memoryTypeCount) return false;
    VkMemoryType* memoryType = &memoryTypes[memoryTypeIndex];
    return memoryType->propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
}

void injectExtensions(VkContext* context, char*** extensions, uint32_t* extensionCount, const char* const* extraExtensions, uint32_t extraExtensionCount, const char* const* skipExtensions, uint32_t skipExtensionCount) {
    if (skipExtensions) {
        uint32_t oldExtensionCount = *extensionCount;
        char** oldExtensions = *extensions;

        bool skipped[oldExtensionCount];
        uint32_t newExtensionCount = 0;
        for (int i = 0; i < oldExtensionCount; i++) {
            skipped[i] = false;
            for (int j = 0; j < skipExtensionCount; j++) {
                if (strcmp(skipExtensions[j], oldExtensions[i]) == 0) {
                    skipped[i] = true;
                    break;
                }
            }

            if (!skipped[i]) newExtensionCount++;
        }

        char** newExtensions = vt_alloc(&context->memoryPool, newExtensionCount * sizeof(char*));
        for (int i = 0, j = 0; i < oldExtensionCount; i++) if (!skipped[i]) newExtensions[j++] = oldExtensions[i];

        *extensions = newExtensions;
        *extensionCount = newExtensionCount;
    }

    uint32_t newExtensionCount = *extensionCount;
    bool founds[extraExtensionCount];
    for (int i = 0; i < extraExtensionCount; i++) {
        founds[i] = false;
        for (int j = 0; j < *extensionCount; j++) {
            if (strcmp(extraExtensions[i], (*extensions)[j]) == 0) {
                founds[i] = true;
                break;
            }
        }

        if (!founds[i]) newExtensionCount++;
    }

    if (newExtensionCount == *extensionCount) return;

    char** newExtensions = vt_alloc(&context->memoryPool, newExtensionCount * sizeof(char*));
    for (int i = 0, j = 0; i < extraExtensionCount; i++) {
        if (!founds[i]) {
            char* value = vt_alloc(&context->memoryPool, strlen(extraExtensions[i]) + 1);
            strcpy(value, extraExtensions[i]);
            newExtensions[(*extensionCount) + j++] = value;
        }
    }

    for (int i = 0; i < *extensionCount; i++) newExtensions[i] = (*extensions)[i];

    *extensions = newExtensions;
    *extensionCount = newExtensionCount;
}

void injectExtensions2(VkContext* context, VkExtensionProperties** extensions, uint32_t* extensionCount, const char* const* extraExtensions, uint32_t extraExtensionCount, const char* const* skipExtensions, uint32_t skipExtensionCount) {
    if (skipExtensions) {
        uint32_t oldExtensionCount = *extensionCount;
        VkExtensionProperties* oldExtensions = *extensions;

        bool skipped[oldExtensionCount];
        uint32_t newExtensionCount = 0;
        for (int i = 0; i < oldExtensionCount; i++) {
            skipped[i] = false;
            for (int j = 0; j < skipExtensionCount; j++) {
                if (strcmp(skipExtensions[j], oldExtensions[i].extensionName) == 0) {
                    skipped[i] = true;
                    break;
                }
            }

            if (!skipped[i]) newExtensionCount++;
        }

        VkExtensionProperties* newExtensions = vt_alloc(&context->memoryPool, newExtensionCount * sizeof(VkExtensionProperties));
        for (int i = 0, j = 0; i < oldExtensionCount; i++) {
            if (!skipped[i]) memcpy(&newExtensions[j++], &oldExtensions[i], sizeof(VkExtensionProperties));
        }

        *extensions = newExtensions;
        *extensionCount = newExtensionCount;
    }

    uint32_t newExtensionCount = *extensionCount;
    bool founds[extraExtensionCount];
    for (int i = 0; i < extraExtensionCount; i++) {
        founds[i] = false;
        for (int j = 0; j < *extensionCount; j++) {
            if (strcmp(extraExtensions[i], (*extensions)[j].extensionName) == 0) {
                founds[i] = true;
                break;
            }
        }

        if (!founds[i]) newExtensionCount++;
    }

    if (newExtensionCount == *extensionCount) return;

    VkExtensionProperties* newExtensions = vt_alloc(&context->memoryPool, newExtensionCount * sizeof(VkExtensionProperties));
    for (int i = 0, j = 0; i < extraExtensionCount; i++) {
        if (!founds[i]) {
            VkExtensionProperties* properties = &newExtensions[(*extensionCount) + j++];
            strcpy(properties->extensionName, extraExtensions[i]);
            properties->specVersion = 1;
        }
    }

    for (int i = 0; i < *extensionCount; i++) memcpy(&newExtensions[i], &(*extensions)[i], sizeof(VkExtensionProperties));

    *extensions = newExtensions;
    *extensionCount = newExtensionCount;
}

void overrideMemoryHeapSize(VkContext* context, VkPhysicalDeviceMemoryProperties* memoryProperties) {
    VkDeviceSize maxHeapSize = (VkDeviceSize)context->maxDeviceMemory << 20;
    for (int i = 0; i < memoryProperties->memoryHeapCount; i++) {
        memoryProperties->memoryHeaps[i].size = MIN(memoryProperties->memoryHeaps[i].size, maxHeapSize);
    }
}

void disableUnsupportedDeviceFeatures(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* createInfo) {
    VkPhysicalDeviceTransformFeedbackFeaturesEXT* transformFeedbackFeatures = findNextVkStructure(createInfo->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TRANSFORM_FEEDBACK_FEATURES_EXT);
    if (transformFeedbackFeatures) {
        VkPhysicalDeviceFeatures2 features = {0};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = transformFeedbackFeatures;
        vulkanWrapper.vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    }

    VkPhysicalDeviceFeatures* enabledFeatures = (VkPhysicalDeviceFeatures*)createInfo->pEnabledFeatures;
    VkPhysicalDeviceFeatures2* features2 = NULL;
    if (!enabledFeatures) {
        features2 = findNextVkStructure((void*)createInfo->pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
        if (features2) enabledFeatures = &features2->features;
    }

    if (enabledFeatures) {
        VkPhysicalDeviceFeatures supportedFeatures = {0};
        vulkanWrapper.vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

        for (int offset = 0; offset < sizeof(VkPhysicalDeviceFeatures); offset += sizeof(VkBool32)) {
            VkBool32 srcValue = *(VkBool32*)(((char*)&supportedFeatures) + offset);
            VkBool32 dstValue = *(VkBool32*)(((char*)enabledFeatures) + offset);
            if (dstValue && !srcValue) *(VkBool32*)(((char*)enabledFeatures) + offset) = VK_FALSE;
        }
    }
}

VkExtensionProperties* getExposedDeviceExtensionProperties(VkContext* context, VkPhysicalDevice physicalDevice, uint32_t* propertyCount) {
    uint32_t srcPropertyCount;
    VkResult result = vulkanWrapper.vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &srcPropertyCount, NULL);
    if (result != VK_SUCCESS) return NULL;

    VkExtensionProperties srcProperties[srcPropertyCount];
    result = vulkanWrapper.vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &srcPropertyCount, srcProperties);
    if (result != VK_SUCCESS) return NULL;

    int dstPropertyCount = context->exposedDeviceExtensions ? context->exposedDeviceExtensions->size : srcPropertyCount;
    VkExtensionProperties* dstProperties = vt_alloc(&context->memoryPool, dstPropertyCount * sizeof(VkExtensionProperties));
    *propertyCount = 0;

    for (int i = 0, j; i < dstPropertyCount; i++) {
        bool expose = true;
        VkExtensionProperties* srcProperty = NULL;
        if (context->exposedDeviceExtensions) {
            expose = false;
            char* extensionName = context->exposedDeviceExtensions->elements[i];

            for (j = 0; j < srcPropertyCount; j++) {
                if (strcmp(extensionName, srcProperties[j].extensionName) == 0) {
                    expose = true;
                    srcProperty = &srcProperties[j];
                    break;
                }
            }
        }
        else srcProperty = &srcProperties[i];
        if (!expose) continue;

        if (context->disabledDeviceExtensions) {
            for (j = 0; j < context->disabledDeviceExtensions->size; j++) {
                char* extensionName = context->disabledDeviceExtensions->elements[j];
                if (strcmp(srcProperty->extensionName, extensionName) == 0) {
                    expose = false;
                    break;
                }
            }
        }

        if (expose) {
            int index = (*propertyCount)++;
            strcpy(dstProperties[index].extensionName, srcProperty->extensionName);
            dstProperties[index].specVersion = srcProperty->specVersion;
        }
    }

    return dstProperties;
}

void checkFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* formatProperties) {
    if (isCompressedFormat(format) && formatProperties->linearTilingFeatures == 0 && formatProperties->optimalTilingFeatures == 0) {
        vulkanWrapper.vkGetPhysicalDeviceFormatProperties(physicalDevice, DECOMPRESSED_FORMAT, formatProperties);
    }
    else if (isFormatScaled(format)) {
        formatProperties->bufferFeatures |= VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT;
    }
}

void checkDeviceProperties(VkContext* context, VkPhysicalDeviceProperties* properties, void* pNext) {
    char deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE] = {0};
    sprintf(deviceName, DEVICE_NAME, properties->deviceName);
    strcpy(properties->deviceName, deviceName);
    properties->apiVersion = context->vkMaxVersion;

    VkPhysicalDeviceVulkan12Properties* vulkan12Properties = findNextVkStructure(pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES);
    if (vulkan12Properties) {
        VK_MAKE_VERSION_STR(vulkan12Properties->driverInfo, properties->driverVersion);

        if (context->disabledDeviceExtensions) {
            bool floatControlsEnabled = true;
            for (int i = 0; i < context->disabledDeviceExtensions->size && floatControlsEnabled; i++) {
                if (strcmp((const char*)context->disabledDeviceExtensions->elements[i], "VK_KHR_shader_float_controls") == 0) floatControlsEnabled = false;
            }

            if (!floatControlsEnabled) {
                vulkan12Properties->shaderSignedZeroInfNanPreserveFloat32 = VK_FALSE;
                vulkan12Properties->shaderSignedZeroInfNanPreserveFloat64 = VK_FALSE;
                vulkan12Properties->shaderDenormFlushToZeroFloat32 = VK_FALSE;
                vulkan12Properties->shaderDenormFlushToZeroFloat64 = VK_FALSE;
                vulkan12Properties->shaderDenormPreserveFloat32 = VK_FALSE;
                vulkan12Properties->shaderDenormPreserveFloat64 = VK_FALSE;
            }
        }
    }

    VkPhysicalDeviceDriverProperties* driverProperties = findNextVkStructure(pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES);
    if (driverProperties) VK_MAKE_VERSION_STR(driverProperties->driverInfo, properties->driverVersion);
}

void checkDeviceFeatures(VkPhysicalDeviceFeatures* features, void* pNext) {
    features->textureCompressionBC = VK_TRUE;
    features->depthClamp = VK_TRUE;
    features->depthBiasClamp = VK_TRUE;
    features->fillModeNonSolid = VK_TRUE;
    features->sampleRateShading = VK_TRUE;
    features->samplerAnisotropy = VK_TRUE;
    features->shaderClipDistance = VK_TRUE;
    features->shaderCullDistance = VK_TRUE;
    features->occlusionQueryPrecise = VK_TRUE;
    features->independentBlend = VK_TRUE;
    features->multiViewport = VK_TRUE;
    features->fullDrawIndexUint32 = VK_TRUE;
    features->logicOp = VK_TRUE;
    features->shaderImageGatherExtended = VK_TRUE;
    features->variableMultisampleRate = VK_TRUE;
    features->dualSrcBlend = VK_TRUE;
    features->imageCubeArray = VK_TRUE;
    features->drawIndirectFirstInstance = VK_TRUE;
    features->fragmentStoresAndAtomics = VK_TRUE;
    features->multiDrawIndirect = VK_TRUE;
    features->tessellationShader = VK_TRUE;
}

void destroyVkObject(VkObjectType type, VkDevice device, void* handle) {
    if (!handle) return;
    switch (type) {
        case VK_OBJECT_TYPE_INSTANCE:
            vulkanWrapper.vkDestroyInstance(handle, NULL);
            break;
        case VK_OBJECT_TYPE_DEVICE:
            vulkanWrapper.vkDestroyDevice(handle, NULL);
            break;
        case VK_OBJECT_TYPE_SEMAPHORE:
            vulkanWrapper.vkDestroySemaphore(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_FENCE:
            vulkanWrapper.vkDestroyFence(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_BUFFER:
            vulkanWrapper.vkDestroyBuffer(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_IMAGE:
            vulkanWrapper.vkDestroyImage(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_EVENT:
            vulkanWrapper.vkDestroyEvent(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_QUERY_POOL:
            vulkanWrapper.vkDestroyQueryPool(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_BUFFER_VIEW:
            vulkanWrapper.vkDestroyBufferView(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_IMAGE_VIEW:
            vulkanWrapper.vkDestroyImageView(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_SHADER_MODULE: {
            ShaderModule* shaderModule = handle;
            vulkanWrapper.vkDestroyShaderModule(device, shaderModule->module, NULL);
            MEMFREE(shaderModule->code);
            MEMFREE(shaderModule);
            break;
        }
        case VK_OBJECT_TYPE_PIPELINE_CACHE:
            vulkanWrapper.vkDestroyPipelineCache(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
            vulkanWrapper.vkDestroyPipelineLayout(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_RENDER_PASS:
            vulkanWrapper.vkDestroyRenderPass(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_PIPELINE: {
            VkPipeline pipeline = AsyncPipelineCreator_getVkHandle(handle);
            if (pipeline) vulkanWrapper.vkDestroyPipeline(device, pipeline, NULL);
            break;
        }
        case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
            vulkanWrapper.vkDestroyDescriptorSetLayout(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_SAMPLER:
            vulkanWrapper.vkDestroySampler(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
            vulkanWrapper.vkDestroyDescriptorPool(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_FRAMEBUFFER:
            vulkanWrapper.vkDestroyFramebuffer(device, handle, NULL);
            break;
        case VK_OBJECT_TYPE_COMMAND_POOL:
            vulkanWrapper.vkDestroyCommandPool(device, handle, NULL);
            break;
    }
}