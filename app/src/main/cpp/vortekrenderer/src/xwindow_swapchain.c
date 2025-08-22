#include "xwindow_swapchain.h"
#include "vulkan_helper.h"

void getWindowExtent(JMethods* jmethods, int windowId, VkExtent2D* extent) {
    extent->width = (*jmethods->env)->CallIntMethod(jmethods->env, jmethods->obj, jmethods->getWindowWidth, windowId);
    extent->height = (*jmethods->env)->CallIntMethod(jmethods->env, jmethods->obj, jmethods->getWindowHeight, windowId);
}

static AHardwareBuffer* getWindowHardwareBuffer(JMethods* jmethods, int windowId) {
    jlong hardwareBufferPtr = (*jmethods->env)->CallLongMethod(jmethods->env, jmethods->obj, jmethods->getWindowHardwareBuffer, windowId);
    return (AHardwareBuffer*)hardwareBufferPtr;
}

static VkResult createImageMemory(VkDevice device, XWindowSwapchain* swapchain, VkImage image, VkDeviceMemory* pMemory) {
    AHardwareBuffer* hardwareBuffer = getWindowHardwareBuffer(swapchain->jmethods, swapchain->windowId);

    VkAndroidHardwareBufferPropertiesANDROID ahbProperties = {0};
    ahbProperties.sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    vulkanWrapper.vkGetAndroidHardwareBufferPropertiesANDROID(device, hardwareBuffer, &ahbProperties);

    VkImportAndroidHardwareBufferInfoANDROID memoryImportInfo = {0};
    memoryImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    memoryImportInfo.buffer = hardwareBuffer;

    VkMemoryDedicatedAllocateInfo memoryDedicatedInfo = {0};
    memoryDedicatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    memoryDedicatedInfo.pNext = &memoryImportInfo;
    memoryDedicatedInfo.image = image;
    memoryDedicatedInfo.buffer = VK_NULL_HANDLE;

    VkMemoryAllocateInfo memoryInfo = {0};
    memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.pNext = &memoryDedicatedInfo;
    memoryInfo.allocationSize = ahbProperties.allocationSize;
    memoryInfo.memoryTypeIndex = getMemoryTypeIndex(ahbProperties.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory memory;
    VkResult result = vulkanWrapper.vkAllocateMemory(device, &memoryInfo, NULL, &memory);
    if (result != VK_SUCCESS) return result;

    result = vulkanWrapper.vkBindImageMemory(device, image, memory, 0);
    if (result != VK_SUCCESS) return result;

    *pMemory = memory;
    return VK_SUCCESS;
}

static VkResult createImage(VkDevice device, XWindowSwapchain* swapchain, XWindowSwapchain_Image* swapchainImage) {
    AHardwareBuffer* hardwareBuffer = getWindowHardwareBuffer(swapchain->jmethods, swapchain->windowId);

    AHardwareBuffer_Desc ahbDesc = {0};
    AHardwareBuffer_describe(hardwareBuffer, &ahbDesc);

    VkExternalFormatANDROID externalFormatAndroid = {0};
    externalFormatAndroid.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormatAndroid.externalFormat = 0;

    VkExternalMemoryImageCreateInfo externalMemoryImageInfo = {0};
    externalMemoryImageInfo.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
    externalMemoryImageInfo.pNext = &externalFormatAndroid;
    externalMemoryImageInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID;

    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = &externalMemoryImageInfo;
    imageInfo.flags = VK_IMAGE_CREATE_ALIAS_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = swapchain->imageFormat;
    imageInfo.extent.width = ahbDesc.width;
    imageInfo.extent.height = ahbDesc.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = swapchain->imageUsage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImage image;
    VkDeviceMemory memory;
    VkResult result;

    result = vulkanWrapper.vkCreateImage(device, &imageInfo, NULL, &image);
    if (result != VK_SUCCESS) return result;

    result = createImageMemory(device, swapchain, image, &memory);
    if (result != VK_SUCCESS) return result;

    swapchainImage->image = image;
    swapchainImage->memory = memory;
    return VK_SUCCESS;
}

int getSurfaceMinImageCount() {
    return 1;
}

VkSurfaceFormatKHR* getSurfaceFormats(uint32_t* formatCount) {
    static const VkFormat supportedFormats[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB};
    int supportedFormatCount = ARRAY_SIZE(supportedFormats);
    VkSurfaceFormatKHR* surfaceFormats = calloc(supportedFormatCount, sizeof(VkSurfaceFormatKHR));

    if (formatCount) *formatCount = supportedFormatCount;

    for (int i = 0; i < supportedFormatCount; i++) {
        surfaceFormats[i].format = supportedFormats[i];
        surfaceFormats[i].colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    return surfaceFormats;
}

XWindowSwapchain* XWindowSwapchain_create(VkDevice device, uint32_t graphicsQueueIndex, VkSwapchainCreateInfoKHR* swapchainInfo, JMethods* jmethods, int windowId) {
    XWindowSwapchain* swapchain = calloc(1, sizeof(XWindowSwapchain));
    swapchain->windowId = windowId;
    swapchain->imageCount = swapchainInfo->minImageCount;
    swapchain->images = calloc(swapchain->imageCount, sizeof(XWindowSwapchain_Image));
    swapchain->imageFormat = swapchainInfo->imageFormat;
    swapchain->imageUsage = swapchainInfo->imageUsage;
    memcpy(&swapchain->imageExtent, &swapchainInfo->imageExtent, sizeof(VkExtent2D));
    swapchain->jmethods = jmethods;

    VkResult result;
    for (int i = 0; i < swapchain->imageCount; i++) {
        result = createImage(device, swapchain, &swapchain->images[i]);
        if (result != VK_SUCCESS) goto error;
    }

    vulkanWrapper.vkGetDeviceQueue(device, graphicsQueueIndex, 0, &swapchain->queue);
    return swapchain;

error:
    MEMFREE(swapchain->images);
    MEMFREE(swapchain);
    return NULL;
}

void XWindowSwapchain_destroy(VkDevice device, XWindowSwapchain* swapchain) {
    if (!swapchain) return;
    for (int i = 0; i < swapchain->imageCount; i++) {
        vulkanWrapper.vkDestroyImage(device, swapchain->images[i].image, NULL);
        vulkanWrapper.vkFreeMemory(device, swapchain->images[i].memory, NULL);
    }

    MEMFREE(swapchain->images);
    MEMFREE(swapchain);
}

VkResult XWindowSwapchain_acquireNextImage(XWindowSwapchain* swapchain, uint64_t timeout, VkSemaphore signalSemaphore, VkFence fence, uint32_t* imageIndex) {
    if (signalSemaphore || fence) {
        VkSubmitInfo submitInfo = {0};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        if (signalSemaphore) {
            submitInfo.pSignalSemaphores = &signalSemaphore;
            submitInfo.signalSemaphoreCount = 1;
        }

        VkResult result = vulkanWrapper.vkQueueSubmit(swapchain->queue, 1, &submitInfo, fence);
        if (result == VK_ERROR_DEVICE_LOST) return result;
    }

    VkExtent2D windowSize;
    getWindowExtent(swapchain->jmethods, swapchain->windowId, &windowSize);

    VkResult result = VK_SUCCESS;
    if (swapchain->imageExtent.width != windowSize.width || swapchain->imageExtent.height != windowSize.height) {
        result = VK_ERROR_SURFACE_LOST_KHR;
    }

    *imageIndex = 0;
    return result;
}

void XWindowSwapchain_presentImage(XWindowSwapchain* swapchain) {
    (*swapchain->jmethods->env)->CallVoidMethod(swapchain->jmethods->env, swapchain->jmethods->obj,
                                                swapchain->jmethods->updateWindowContent, swapchain->windowId);
}