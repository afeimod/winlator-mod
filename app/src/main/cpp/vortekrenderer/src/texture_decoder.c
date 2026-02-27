#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "texture_decoder.h"
#include "vulkan_helper.h"
#include "bc_decoder.h"
#include "string_utils.h"
#include "file_utils.h"

#define CACHE_DIR APP_CACHE_DIR "/vortek"
#define CACHE_MIN_IMAGE_WIDTH 1024

static bool isCanDecompressFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

static int indexOfBoundBuffer(TextureDecoder* textureDecoder, VkBuffer buffer) {
    for (int i = 0; i < textureDecoder->boundBuffers.size; i++) {
        TextureDecoder_BoundBuffer* current = textureDecoder->boundBuffers.elements[i];
        if (current->buffer == buffer) return i;
    }
    return -1;
}

static int indexOfImage(TextureDecoder* textureDecoder, VkImage image) {
    for (int i = 0; i < textureDecoder->images.size; i++) {
        TextureDecoder_Image* current = textureDecoder->images.elements[i];
        if (current->image == image) return i;
    }
    return -1;
}

static void getBCInfo(VkFormat format, int* bcN, int* blockSize, bool* isNoAlphaU) {
    *isNoAlphaU = false;
    switch (format) {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            *isNoAlphaU = true;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            *blockSize = 8;
            *bcN = 1;
            break;
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
            *blockSize = 16;
            *bcN = 2;
            break;
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
            *blockSize = 16;
            *bcN = 3;
            break;
        case VK_FORMAT_BC4_UNORM_BLOCK:
            *isNoAlphaU = true;
        case VK_FORMAT_BC4_SNORM_BLOCK:
            *blockSize = 8;
            *bcN = 4;
            break;
        case VK_FORMAT_BC5_UNORM_BLOCK:
            *isNoAlphaU = true;
        case VK_FORMAT_BC5_SNORM_BLOCK:
            *blockSize = 16;
            *bcN = 5;
            break;
        default:
            *blockSize = 0;
            *bcN = 0;
            break;
    }
}

static void internalDestroyImage(VkDevice device, TextureDecoder_Image* targetImage) {
    if (targetImage->decompressedData) vulkanWrapper.vkUnmapMemory(device, targetImage->memory);
    if (targetImage->image) vulkanWrapper.vkDestroyImage(device, targetImage->image, NULL);
    if (targetImage->buffer) vulkanWrapper.vkDestroyBuffer(device, targetImage->buffer, NULL);
    if (targetImage->memory) vulkanWrapper.vkFreeMemory(device, targetImage->memory, NULL);
    MEMFREE(targetImage);
}

static bool readCachedImage(TextureDecoder_Image* image, uint64_t hash) {
    char filename[128] = {0};
    sprintf(filename, CACHE_DIR "/%lx-%dx%d-%d.imd", hash, image->width, image->height, image->format);

    createDirectory(CACHE_DIR);
    size_t size = image->width * image->height * 4;
    return fileGetContents(filename, image->decompressedData, &size) ? true : false;
}

static void writeImageToCache(TextureDecoder* textureDecoder, TextureDecoder_Image* image, uint64_t hash) {
    if (textureDecoder->imageCacheSize == 0) return;
    createDirectory(CACHE_DIR);

    char* content = fileGetContents(CACHE_DIR "/.cache-size", NULL, NULL);
    uint64_t currentCacheSize = 0;
    if (content) {
        currentCacheSize = strtoll(content, NULL, 10);
        MEMFREE(content);
    }

    uint64_t maxCacheSize = (uint64_t)textureDecoder->imageCacheSize << 20;
    while (currentCacheSize > maxCacheSize) {
        FindFileInfo fileInfo = {0};
        if (findFirstFile(CACHE_DIR, &fileInfo) && remove(fileInfo.path) == 0) {
            currentCacheSize -= fileInfo.size;
        }
        else return;
    }

    char filename[128] = {0};
    sprintf(filename, CACHE_DIR "/%lx-%dx%d-%d.imd", hash, image->width, image->height, image->format);
    size_t size = image->width * image->height * 4;

    bool success = false;
    if (filePutContents(filename, image->decompressedData, size)) {
        currentCacheSize += size;
        char value[32] = {0};
        sprintf(value, "%ld", currentCacheSize);
        success = filePutContents(CACHE_DIR "/.cache-size", value, strlen(value));
    }

    if (!success) remove(filename);
}

TextureDecoder* TextureDecoder_create(VkContext* context, VkPhysicalDeviceFeatures* supportedFeatures) {
    if (supportedFeatures->textureCompressionBC) return NULL;
    TextureDecoder* textureDecoder = calloc(1, sizeof(TextureDecoder));
    textureDecoder->imageCacheSize = context->imageCacheSize;
    textureDecoder->threadPool = context->threadPool;
    return textureDecoder;
}

void TextureDecoder_destroy(TextureDecoder* textureDecoder) {
    MEMFREE(textureDecoder->images.elements);
    MEMFREE(textureDecoder->boundBuffers.elements);
    MEMFREE(textureDecoder->bufferImageCopies.elements);
    MEMFREE(textureDecoder);
}

void TextureDecoder_decodeAll(TextureDecoder* textureDecoder) {
    while (!ArrayDeque_isEmpty(&textureDecoder->bufferImageCopies)) {
        TextureDecoder_BufferImageCopy* bufferImageCopy = ArrayDeque_removeFirst(&textureDecoder->bufferImageCopies);
        if (!bufferImageCopy) continue;

        TextureDecoder_BoundBuffer* srcBuffer = bufferImageCopy->srcBuffer;
        TextureDecoder_Image* dstImage = bufferImageCopy->dstImage;
        VkDeviceSize bufferOffset = bufferImageCopy->bufferOffset;
        MEMFREE(bufferImageCopy);

        if (!srcBuffer || !dstImage) continue;

        void* data = mmap(NULL, srcBuffer->memory->allocationSize, PROT_WRITE | PROT_READ, MAP_SHARED, srcBuffer->memory->fd, 0);
        if (data == MAP_FAILED) continue;
        void* imageData = data + (srcBuffer->memoryOffset + bufferOffset);

        int blockSize;
        int bcN;
        bool isNoAlphaU;
        getBCInfo(dstImage->format, &bcN, &blockSize, &isNoAlphaU);

        uint64_t hash = 0;
        bool isCachedImage = false;
        if (dstImage->width >= CACHE_MIN_IMAGE_WIDTH && dstImage->height >= CACHE_MIN_IMAGE_WIDTH) {
            int memorySize = (dstImage->width / 4) * (dstImage->height / 4) * blockSize;
            hash = murmurHash64(imageData, memorySize, dstImage->format);
            isCachedImage = readCachedImage(dstImage, hash);
        }

        if (!isCachedImage) {
            BCDecoder_decode(imageData, dstImage->decompressedData, dstImage->width, dstImage->height, bcN, isNoAlphaU, textureDecoder->threadPool);
            if (hash > 0) writeImageToCache(textureDecoder, dstImage, hash);
        }

        munmap(data, srcBuffer->memory->allocationSize);
    }
}

void TextureDecoder_copyBufferToImage(TextureDecoder* textureDecoder, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, VkDeviceSize bufferOffset) {
    int index = indexOfBoundBuffer(textureDecoder, srcBuffer);
    if (index == -1) return;
    TextureDecoder_BoundBuffer* boundBuffer = textureDecoder->boundBuffers.elements[index];

    index = indexOfImage(textureDecoder, dstImage);
    if (index == -1) return;
    TextureDecoder_Image* targetImage = textureDecoder->images.elements[index];

    TextureDecoder_BufferImageCopy* bufferImageCopy = calloc(1, sizeof(TextureDecoder_BufferImageCopy));
    bufferImageCopy->srcBuffer = boundBuffer;
    bufferImageCopy->dstImage = targetImage;
    bufferImageCopy->bufferOffset = bufferOffset;
    ArrayDeque_addLast(&textureDecoder->bufferImageCopies, bufferImageCopy);

    VkBufferImageCopy region = {0};
    region.imageExtent.width = targetImage->width;
    region.imageExtent.height = targetImage->height;
    region.imageExtent.depth = 1;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    vulkanWrapper.vkCmdCopyBufferToImage(commandBuffer, targetImage->buffer, dstImage, dstImageLayout, 1, &region);
}

bool TextureDecoder_containsImage(TextureDecoder* textureDecoder, VkImage image) {
    return indexOfImage(textureDecoder, image) != -1;
}

VkResult TextureDecoder_createImage(TextureDecoder* textureDecoder, VkDevice device, VkImageCreateInfo* imageInfo, VkImage* pImage) {
    *pImage = VK_NULL_HANDLE;
    VkResult result;
    if (!isCanDecompressFormat(imageInfo->format)) return VK_ERROR_FORMAT_NOT_SUPPORTED;

    TextureDecoder_Image* newImage = calloc(1, sizeof(TextureDecoder_Image));
    newImage->format = imageInfo->format;
    newImage->width = imageInfo->extent.width;
    newImage->height = imageInfo->extent.height;

    imageInfo->format = DECOMPRESSED_FORMAT;
    imageInfo->flags &= ~VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
    imageInfo->mipLevels = 1;

    VkImageFormatListCreateInfo* formatListInfo = findNextVkStructure(imageInfo->pNext, VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO);
    if (formatListInfo && formatListInfo->pViewFormats) {
        formatListInfo->viewFormatCount = 1;
        VkFormat* viewFormats = (VkFormat*)formatListInfo->pViewFormats;
        viewFormats[0] = DECOMPRESSED_FORMAT;
    }

    VkImage image;
    result = vulkanWrapper.vkCreateImage(device, imageInfo, NULL, &image);
    if (result != VK_SUCCESS) goto error;;

    VkBufferCreateInfo imageBufferInfo = {0};
    imageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    imageBufferInfo.size = imageInfo->extent.width * imageInfo->extent.height * 4;
    imageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkBuffer buffer;
    result = vulkanWrapper.vkCreateBuffer(device, &imageBufferInfo, NULL, &buffer);
    if (result != VK_SUCCESS) goto error;

    VkMemoryRequirements memReqs = {0};
    vulkanWrapper.vkGetBufferMemoryRequirements(device, buffer, &memReqs);

    VkMemoryAllocateInfo allocateInfo = {0};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memReqs.size;
    allocateInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDeviceMemory memory;
    result = vulkanWrapper.vkAllocateMemory(device, &allocateInfo, NULL, &memory);
    if (result != VK_SUCCESS) goto error;

    result = vulkanWrapper.vkBindBufferMemory(device, buffer, memory, 0);
    if (result != VK_SUCCESS) goto error;

    int imageSize = newImage->width * newImage->height * 4;
    result = vulkanWrapper.vkMapMemory(device, memory, 0, imageSize, 0, &newImage->decompressedData);
    if (result != VK_SUCCESS) goto error;
    memset(newImage->decompressedData, 0, imageSize);

    newImage->image = image;
    newImage->buffer = buffer;
    newImage->memory = memory;
    ArrayList_add(&textureDecoder->images, newImage);

    *pImage = image;
    return result;

error:
    internalDestroyImage(device, newImage);
    return result;
}

void TextureDecoder_destroyImage(TextureDecoder* textureDecoder, VkDevice device, VkImage image) {
    int index = indexOfImage(textureDecoder, image);
    if (index != -1) {
        TextureDecoder_Image* targetImage = textureDecoder->images.elements[index];
        internalDestroyImage(device, targetImage);
        ArrayList_removeAt(&textureDecoder->images, index);
    }
}

void TextureDecoder_addBoundBuffer(TextureDecoder* textureDecoder, ResourceMemory* memory, VkBuffer buffer, VkDeviceSize memoryOffset) {
    if (memory->fd <= 0) return;
    TextureDecoder_removeBoundBuffer(textureDecoder, buffer);

    TextureDecoder_BoundBuffer* boundBuffer = calloc(1, sizeof(TextureDecoder_BoundBuffer));
    boundBuffer->buffer = buffer;
    boundBuffer->memoryOffset = memoryOffset;
    boundBuffer->memory = memory;

    ArrayList_add(&textureDecoder->boundBuffers, boundBuffer);
}

void TextureDecoder_removeBoundBuffer(TextureDecoder* textureDecoder, VkBuffer buffer) {
    int index = indexOfBoundBuffer(textureDecoder, buffer);
    if (index != -1) {
        TextureDecoder_BoundBuffer* boundBuffer = textureDecoder->boundBuffers.elements[index];
        MEMFREE(boundBuffer);
        ArrayList_removeAt(&textureDecoder->boundBuffers, index);
    }
}

bool isCompressedFormat(VkFormat format) {
    switch(format) {
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return true;
        default:
            return false;
    }
}

VkResult getCompressedImageFormatProperties(VkFormat format, VkImageFormatProperties* pImageFormatProperties) {
    if (isCanDecompressFormat(format)) {
        pImageFormatProperties->maxExtent.width = 16384;
        pImageFormatProperties->maxExtent.height = 16384;
        pImageFormatProperties->maxExtent.depth = 1;
        pImageFormatProperties->maxMipLevels = 15;
        pImageFormatProperties->maxArrayLayers = 2048;
        pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
        pImageFormatProperties->maxResourceSize = 1u << 31;
        return VK_SUCCESS;
    }
    else return VK_ERROR_FORMAT_NOT_SUPPORTED;
}