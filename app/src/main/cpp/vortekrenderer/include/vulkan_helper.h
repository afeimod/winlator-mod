#ifndef VULKAN_HELPER_H
#define VULKAN_HELPER_H

#include "vortek.h"
#include "vk_context.h"
#include "vulkan_wrapper.h"

#if ENABLE_VALIDATION_LAYER
static const char* validationLayers[] = {"VK_LAYER_GOOGLE_threading",  "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker", "VK_LAYER_LUNARG_core_validation", "VK_LAYER_KHRONOS_validation", "VK_LAYER_GOOGLE_unique_objects"};
#endif

static const char* globalExposedDeviceExtensions[] = {"VK_KHR_surface", "VK_KHR_swapchain", "VK_KHR_get_physical_device_properties2", "VK_EXT_transform_feedback", "VK_EXT_conditional_rendering", "VK_EXT_vertex_attribute_divisor", "VK_EXT_index_type_uint8", "VK_EXT_robustness2", "VK_EXT_extended_dynamic_state", "VK_EXT_host_query_reset", "VK_KHR_create_renderpass2", "VK_KHR_depth_stencil_resolve", "VK_KHR_draw_indirect_count", "VK_KHR_timeline_semaphore", "VK_KHR_dedicated_allocation", "VK_KHR_get_memory_requirements2", "VK_KHR_descriptor_update_template", "VK_KHR_imageless_framebuffer", "VK_KHR_driver_properties", "VK_KHR_image_format_list", "VK_EXT_shader_demote_to_helper_invocation", "VK_KHR_shader_float_controls", "VK_EXT_4444_formats", "VK_EXT_conservative_rasterization", "VK_EXT_custom_border_color", "VK_EXT_depth_clip_enable", "VK_EXT_sample_locations", "VK_KHR_sampler_ycbcr_conversion", "VK_EXT_provoking_vertex", "VK_KHR_maintenance1", "VK_KHR_maintenance2", "VK_KHR_maintenance3", "VK_EXT_line_rasterization", "VK_EXT_border_color_swizzle", "VK_KHR_external_memory", "VK_KHR_external_memory_fd", "VK_KHR_external_fence", "VK_KHR_external_fence_fd", "VK_KHR_external_semaphore", "VK_KHR_external_semaphore_fd", "VK_KHR_vulkan_memory_model", "VK_KHR_synchronization2", "VK_EXT_depth_clip_control", "VK_KHR_dynamic_rendering", "VK_KHR_shader_float16_int8", "VK_KHR_push_descriptor", "VK_EXT_shader_stencil_export", "VK_EXT_shader_viewport_index_layer", "VK_KHR_sampler_mirror_clamp_to_edge", "VK_KHR_shader_draw_parameters", "VK_EXT_scalar_block_layout", "VK_EXT_color_write_enable", "VK_EXT_extended_dynamic_state3", "VK_EXT_shader_module_identifier", "VK_KHR_portability_subset"};
static const char* globalImplementedDeviceExtensions[] = {"VK_KHR_swapchain", "VK_KHR_descriptor_update_template", "VK_EXT_private_data", "VK_EXT_memory_budget"};

extern VulkanWrapper vulkanWrapper;

extern void initVulkanInstance(VkContext* context, VkInstance instance, const VkApplicationInfo* applicationInfo);
extern void initVulkanDevice(VkContext* context, VkPhysicalDevice physicalDevice, VkDevice device);
extern uint32_t getMemoryTypeIndex(uint32_t typeBits, VkFlags properties);
extern uint32_t getMemoryPropertyFlags(uint32_t memoryTypeIndex);
extern bool isHostVisibleMemory(uint32_t memoryTypeIndex);
extern void injectExtensions(VkContext* context, char*** extensions, uint32_t* extensionCount, const char* const* extraExtensions, uint32_t extraExtensionCount, const char* const* skipExtensions, uint32_t skipExtensionCount);
extern void injectExtensions2(VkContext* context, VkExtensionProperties** extensions, uint32_t* extensionCount, const char* const* extraExtensions, uint32_t extraExtensionCount, const char* const* skipExtensions, uint32_t skipExtensionCount);
extern void overrideMemoryHeapSize(VkContext* context, VkPhysicalDeviceMemoryProperties* memoryProperties);
extern void disableUnsupportedDeviceFeatures(VkPhysicalDevice physicalDevice, VkDeviceCreateInfo* createInfo);
extern VkExtensionProperties* getExposedDeviceExtensionProperties(VkContext* context, VkPhysicalDevice physicalDevice, uint32_t* propertyCount);
extern void checkFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* formatProperties);
extern void checkDeviceProperties(VkContext* context, VkPhysicalDeviceProperties* properties, void* pNext);
extern void checkDeviceFeatures(VkPhysicalDeviceFeatures* features, void* pNext);
extern void destroyVkObject(VkObjectType type, VkDevice device, void* handle);

#endif
