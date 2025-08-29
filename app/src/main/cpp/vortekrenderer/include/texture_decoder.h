#ifndef VORTEK_TEXTURE_DECODER_H
#define VORTEK_TEXTURE_DECODER_H

#include "vortek.h"

typedef struct TextureDecoder_Image {
    VkImage image;
    VkFormat format;
    short width;
    short height;
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* decompressedData;
} TextureDecoder_Image;

typedef struct TextureDecoder_BoundBuffer {
    VkBuffer buffer;
    VkDeviceSize memoryOffset;
    ResourceMemory* memory;
} TextureDecoder_BoundBuffer;

typedef struct TextureDecoder_BufferImageCopy {
    TextureDecoder_BoundBuffer* srcBuffer;
    TextureDecoder_Image* dstImage;
    VkDeviceSize bufferOffset;
} TextureDecoder_BufferImageCopy;

typedef struct TextureDecoder {
    ArrayList images;
    ArrayList boundBuffers;
    ArrayDeque bufferImageCopies;
    short imageCacheSize;
    ThreadPool* threadPool;
} TextureDecoder;

extern TextureDecoder* TextureDecoder_create(VkContext* context, VkPhysicalDeviceFeatures* supportedFeatures);
extern void TextureDecoder_destroy(TextureDecoder* textureDecoder);
extern void TextureDecoder_decodeAll(TextureDecoder* textureDecoder);
extern void TextureDecoder_copyBufferToImage(TextureDecoder* textureDecoder, VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, VkDeviceSize bufferOffset);
extern bool TextureDecoder_containsImage(TextureDecoder* textureDecoder, VkImage image);
extern VkResult TextureDecoder_createImage(TextureDecoder* textureDecoder, VkDevice device, VkImageCreateInfo* imageInfo, VkImage* pImage);
extern void TextureDecoder_destroyImage(TextureDecoder* textureDecoder, VkDevice device, VkImage image);
extern void TextureDecoder_addBoundBuffer(TextureDecoder* textureDecoder, ResourceMemory* resourceMemory, VkBuffer buffer, VkDeviceSize memoryOffset);
extern void TextureDecoder_removeBoundBuffer(TextureDecoder* textureDecoder, VkBuffer buffer);
extern bool isCompressedFormat(VkFormat format);
extern VkResult getCompressedImageFormatProperties(VkFormat format, VkImageFormatProperties* pImageFormatProperties);

#endif

#define DECOMPRESSED_FORMAT VK_FORMAT_B8G8R8A8_UNORM