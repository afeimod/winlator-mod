#ifndef VORTEK_REQUEST_CODES_H
#define VORTEK_REQUEST_CODES_H

#include "vortek.h"

#define REQUEST_CODE_CREATE_CONTEXT 1
#define REQUEST_CODE_SEND_EXTRA_DATA 2

#define REQUEST_CODE_VK_CALL_START 100
#define REQUEST_CODE_VK_CALL_COUNT 254

#define REQUEST_CODE_VK_CREATE_INSTANCE 100
#define REQUEST_CODE_VK_DESTROY_INSTANCE 101
#define REQUEST_CODE_VK_ENUMERATE_PHYSICAL_DEVICES 102
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_PROPERTIES 103
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_QUEUE_FAMILY_PROPERTIES 104
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_MEMORY_PROPERTIES 105
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_FEATURES 106
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_FORMAT_PROPERTIES 107
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_IMAGE_FORMAT_PROPERTIES 108
#define REQUEST_CODE_VK_CREATE_DEVICE 109
#define REQUEST_CODE_VK_DESTROY_DEVICE 110
#define REQUEST_CODE_VK_ENUMERATE_INSTANCE_VERSION 111
#define REQUEST_CODE_VK_ENUMERATE_INSTANCE_EXTENSION_PROPERTIES 112
#define REQUEST_CODE_VK_ENUMERATE_DEVICE_EXTENSION_PROPERTIES 113
#define REQUEST_CODE_VK_GET_DEVICE_QUEUE 114
#define REQUEST_CODE_VK_QUEUE_SUBMIT 115
#define REQUEST_CODE_VK_QUEUE_WAIT_IDLE 116
#define REQUEST_CODE_VK_DEVICE_WAIT_IDLE 117
#define REQUEST_CODE_VK_ALLOCATE_MEMORY 118
#define REQUEST_CODE_VK_FREE_MEMORY 119
#define REQUEST_CODE_VK_MAP_MEMORY 120
#define REQUEST_CODE_VK_UNMAP_MEMORY 121
#define REQUEST_CODE_VK_FLUSH_MAPPED_MEMORY_RANGES 122
#define REQUEST_CODE_VK_INVALIDATE_MAPPED_MEMORY_RANGES 123
#define REQUEST_CODE_VK_GET_DEVICE_MEMORY_COMMITMENT 124
#define REQUEST_CODE_VK_GET_BUFFER_MEMORY_REQUIREMENTS 125
#define REQUEST_CODE_VK_BIND_BUFFER_MEMORY 126
#define REQUEST_CODE_VK_GET_IMAGE_MEMORY_REQUIREMENTS 127
#define REQUEST_CODE_VK_BIND_IMAGE_MEMORY 128
#define REQUEST_CODE_VK_GET_IMAGE_SPARSE_MEMORY_REQUIREMENTS 129
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_PROPERTIES 130
#define REQUEST_CODE_VK_QUEUE_BIND_SPARSE 131
#define REQUEST_CODE_VK_CREATE_FENCE 132
#define REQUEST_CODE_VK_DESTROY_FENCE 133
#define REQUEST_CODE_VK_RESET_FENCES 134
#define REQUEST_CODE_VK_GET_FENCE_STATUS 135
#define REQUEST_CODE_VK_WAIT_FOR_FENCES 136
#define REQUEST_CODE_VK_CREATE_SEMAPHORE 137
#define REQUEST_CODE_VK_DESTROY_SEMAPHORE 138
#define REQUEST_CODE_VK_CREATE_EVENT 139
#define REQUEST_CODE_VK_DESTROY_EVENT 140
#define REQUEST_CODE_VK_GET_EVENT_STATUS 141
#define REQUEST_CODE_VK_SET_EVENT 142
#define REQUEST_CODE_VK_RESET_EVENT 143
#define REQUEST_CODE_VK_CREATE_QUERY_POOL 144
#define REQUEST_CODE_VK_DESTROY_QUERY_POOL 145
#define REQUEST_CODE_VK_GET_QUERY_POOL_RESULTS 146
#define REQUEST_CODE_VK_RESET_QUERY_POOL 147
#define REQUEST_CODE_VK_CREATE_BUFFER 148
#define REQUEST_CODE_VK_DESTROY_BUFFER 149
#define REQUEST_CODE_VK_CREATE_BUFFER_VIEW 150
#define REQUEST_CODE_VK_DESTROY_BUFFER_VIEW 151
#define REQUEST_CODE_VK_CREATE_IMAGE 152
#define REQUEST_CODE_VK_DESTROY_IMAGE 153
#define REQUEST_CODE_VK_GET_IMAGE_SUBRESOURCE_LAYOUT 154
#define REQUEST_CODE_VK_CREATE_IMAGE_VIEW 155
#define REQUEST_CODE_VK_DESTROY_IMAGE_VIEW 156
#define REQUEST_CODE_VK_CREATE_SHADER_MODULE 157
#define REQUEST_CODE_VK_DESTROY_SHADER_MODULE 158
#define REQUEST_CODE_VK_CREATE_PIPELINE_CACHE 159
#define REQUEST_CODE_VK_DESTROY_PIPELINE_CACHE 160
#define REQUEST_CODE_VK_GET_PIPELINE_CACHE_DATA 161
#define REQUEST_CODE_VK_MERGE_PIPELINE_CACHES 162
#define REQUEST_CODE_VK_CREATE_GRAPHICS_PIPELINES 163
#define REQUEST_CODE_VK_CREATE_COMPUTE_PIPELINES 164
#define REQUEST_CODE_VK_DESTROY_PIPELINE 165
#define REQUEST_CODE_VK_CREATE_PIPELINE_LAYOUT 166
#define REQUEST_CODE_VK_DESTROY_PIPELINE_LAYOUT 167
#define REQUEST_CODE_VK_CREATE_SAMPLER 168
#define REQUEST_CODE_VK_DESTROY_SAMPLER 169
#define REQUEST_CODE_VK_CREATE_DESCRIPTOR_SET_LAYOUT 170
#define REQUEST_CODE_VK_DESTROY_DESCRIPTOR_SET_LAYOUT 171
#define REQUEST_CODE_VK_CREATE_DESCRIPTOR_POOL 172
#define REQUEST_CODE_VK_DESTROY_DESCRIPTOR_POOL 173
#define REQUEST_CODE_VK_RESET_DESCRIPTOR_POOL 174
#define REQUEST_CODE_VK_ALLOCATE_DESCRIPTOR_SETS 175
#define REQUEST_CODE_VK_FREE_DESCRIPTOR_SETS 176
#define REQUEST_CODE_VK_UPDATE_DESCRIPTOR_SETS 177
#define REQUEST_CODE_VK_CREATE_FRAMEBUFFER 178
#define REQUEST_CODE_VK_DESTROY_FRAMEBUFFER 179
#define REQUEST_CODE_VK_CREATE_RENDER_PASS 180
#define REQUEST_CODE_VK_DESTROY_RENDER_PASS 181
#define REQUEST_CODE_VK_GET_RENDER_AREA_GRANULARITY 182
#define REQUEST_CODE_VK_CREATE_COMMAND_POOL 183
#define REQUEST_CODE_VK_DESTROY_COMMAND_POOL 184
#define REQUEST_CODE_VK_RESET_COMMAND_POOL 185
#define REQUEST_CODE_VK_ALLOCATE_COMMAND_BUFFERS 186
#define REQUEST_CODE_VK_FREE_COMMAND_BUFFERS 187
#define REQUEST_CODE_VK_BEGIN_COMMAND_BUFFER 188
#define REQUEST_CODE_VK_END_COMMAND_BUFFER 189
#define REQUEST_CODE_VK_RESET_COMMAND_BUFFER 190
#define REQUEST_CODE_VK_CMD_BIND_PIPELINE 191
#define REQUEST_CODE_VK_CMD_SET_VIEWPORT 192
#define REQUEST_CODE_VK_CMD_SET_SCISSOR 193
#define REQUEST_CODE_VK_CMD_SET_LINE_WIDTH 194
#define REQUEST_CODE_VK_CMD_SET_DEPTH_BIAS 195
#define REQUEST_CODE_VK_CMD_SET_BLEND_CONSTANTS 196
#define REQUEST_CODE_VK_CMD_SET_DEPTH_BOUNDS 197
#define REQUEST_CODE_VK_CMD_SET_STENCIL_COMPARE_MASK 198
#define REQUEST_CODE_VK_CMD_SET_STENCIL_WRITE_MASK 199
#define REQUEST_CODE_VK_CMD_SET_STENCIL_REFERENCE 200
#define REQUEST_CODE_VK_CMD_BIND_DESCRIPTOR_SETS 201
#define REQUEST_CODE_VK_CMD_BIND_INDEX_BUFFER 202
#define REQUEST_CODE_VK_CMD_BIND_VERTEX_BUFFERS 203
#define REQUEST_CODE_VK_CMD_DRAW 204
#define REQUEST_CODE_VK_CMD_DRAW_INDEXED 205
#define REQUEST_CODE_VK_CMD_DRAW_INDIRECT 206
#define REQUEST_CODE_VK_CMD_DRAW_INDEXED_INDIRECT 207
#define REQUEST_CODE_VK_CMD_DISPATCH 208
#define REQUEST_CODE_VK_CMD_DISPATCH_INDIRECT 209
#define REQUEST_CODE_VK_CMD_COPY_BUFFER 210
#define REQUEST_CODE_VK_CMD_COPY_IMAGE 211
#define REQUEST_CODE_VK_CMD_BLIT_IMAGE 212
#define REQUEST_CODE_VK_CMD_COPY_BUFFER_TO_IMAGE 213
#define REQUEST_CODE_VK_CMD_COPY_IMAGE_TO_BUFFER 214
#define REQUEST_CODE_VK_CMD_UPDATE_BUFFER 215
#define REQUEST_CODE_VK_CMD_FILL_BUFFER 216
#define REQUEST_CODE_VK_CMD_CLEAR_COLOR_IMAGE 217
#define REQUEST_CODE_VK_CMD_CLEAR_DEPTH_STENCIL_IMAGE 218
#define REQUEST_CODE_VK_CMD_CLEAR_ATTACHMENTS 219
#define REQUEST_CODE_VK_CMD_RESOLVE_IMAGE 220
#define REQUEST_CODE_VK_CMD_SET_EVENT 221
#define REQUEST_CODE_VK_CMD_RESET_EVENT 222
#define REQUEST_CODE_VK_CMD_WAIT_EVENTS 223
#define REQUEST_CODE_VK_CMD_PIPELINE_BARRIER 224
#define REQUEST_CODE_VK_CMD_BEGIN_QUERY 225
#define REQUEST_CODE_VK_CMD_END_QUERY 226
#define REQUEST_CODE_VK_CMD_BEGIN_CONDITIONAL_RENDERING_EXT 227
#define REQUEST_CODE_VK_CMD_END_CONDITIONAL_RENDERING_EXT 228
#define REQUEST_CODE_VK_CMD_RESET_QUERY_POOL 229
#define REQUEST_CODE_VK_CMD_WRITE_TIMESTAMP 230
#define REQUEST_CODE_VK_CMD_COPY_QUERY_POOL_RESULTS 231
#define REQUEST_CODE_VK_CMD_PUSH_CONSTANTS 232
#define REQUEST_CODE_VK_CMD_BEGIN_RENDER_PASS 233
#define REQUEST_CODE_VK_CMD_NEXT_SUBPASS 234
#define REQUEST_CODE_VK_CMD_END_RENDER_PASS 235
#define REQUEST_CODE_VK_CMD_EXECUTE_COMMANDS 236
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_SURFACE_CAPABILITIES_KHR 237
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_SURFACE_FORMATS_KHR 238
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_SURFACE_PRESENT_MODES_KHR 239
#define REQUEST_CODE_VK_CREATE_SWAPCHAIN_KHR 240
#define REQUEST_CODE_VK_DESTROY_SWAPCHAIN_KHR 241
#define REQUEST_CODE_VK_GET_SWAPCHAIN_IMAGES_KHR 242
#define REQUEST_CODE_VK_ACQUIRE_NEXT_IMAGE_KHR 243
#define REQUEST_CODE_VK_QUEUE_PRESENT_KHR 244
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_FEATURES2 245
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_PROPERTIES2 246
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_FORMAT_PROPERTIES2 247
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_IMAGE_FORMAT_PROPERTIES2 248
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_QUEUE_FAMILY_PROPERTIES2 249
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_MEMORY_PROPERTIES2 250
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_PROPERTIES2 251
#define REQUEST_CODE_VK_CMD_PUSH_DESCRIPTOR_SET_KHR 252
#define REQUEST_CODE_VK_TRIM_COMMAND_POOL 253
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_EXTERNAL_BUFFER_PROPERTIES 254
#define REQUEST_CODE_VK_GET_MEMORY_FD_KHR 255
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_EXTERNAL_SEMAPHORE_PROPERTIES 256
#define REQUEST_CODE_VK_GET_SEMAPHORE_FD_KHR 257
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_EXTERNAL_FENCE_PROPERTIES 258
#define REQUEST_CODE_VK_GET_FENCE_FD_KHR 259
#define REQUEST_CODE_VK_ENUMERATE_PHYSICAL_DEVICE_GROUPS 260
#define REQUEST_CODE_VK_GET_DEVICE_GROUP_PEER_MEMORY_FEATURES 261
#define REQUEST_CODE_VK_BIND_BUFFER_MEMORY2 262
#define REQUEST_CODE_VK_BIND_IMAGE_MEMORY2 263
#define REQUEST_CODE_VK_CMD_SET_DEVICE_MASK 264
#define REQUEST_CODE_VK_ACQUIRE_NEXT_IMAGE2_KHR 265
#define REQUEST_CODE_VK_CMD_DISPATCH_BASE 266
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_PRESENT_RECTANGLES_KHR 267
#define REQUEST_CODE_VK_CMD_SET_SAMPLE_LOCATIONS_EXT 268
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_MULTISAMPLE_PROPERTIES_EXT 269
#define REQUEST_CODE_VK_GET_BUFFER_MEMORY_REQUIREMENTS2 270
#define REQUEST_CODE_VK_GET_IMAGE_MEMORY_REQUIREMENTS2 271
#define REQUEST_CODE_VK_GET_IMAGE_SPARSE_MEMORY_REQUIREMENTS2 272
#define REQUEST_CODE_VK_GET_DEVICE_BUFFER_MEMORY_REQUIREMENTS 273
#define REQUEST_CODE_VK_GET_DEVICE_IMAGE_MEMORY_REQUIREMENTS 274
#define REQUEST_CODE_VK_GET_DEVICE_IMAGE_SPARSE_MEMORY_REQUIREMENTS 275
#define REQUEST_CODE_VK_CREATE_SAMPLER_YCBCR_CONVERSION 276
#define REQUEST_CODE_VK_DESTROY_SAMPLER_YCBCR_CONVERSION 277
#define REQUEST_CODE_VK_GET_DEVICE_QUEUE2 278
#define REQUEST_CODE_VK_GET_DESCRIPTOR_SET_LAYOUT_SUPPORT 279
#define REQUEST_CODE_VK_GET_PHYSICAL_DEVICE_CALIBRATEABLE_TIME_DOMAINS_KHR 280
#define REQUEST_CODE_VK_GET_CALIBRATED_TIMESTAMPS_KHR 281
#define REQUEST_CODE_VK_CREATE_RENDER_PASS2 282
#define REQUEST_CODE_VK_CMD_BEGIN_RENDER_PASS2 283
#define REQUEST_CODE_VK_CMD_NEXT_SUBPASS2 284
#define REQUEST_CODE_VK_CMD_END_RENDER_PASS2 285
#define REQUEST_CODE_VK_GET_SEMAPHORE_COUNTER_VALUE 286
#define REQUEST_CODE_VK_WAIT_SEMAPHORES 287
#define REQUEST_CODE_VK_SIGNAL_SEMAPHORE 288
#define REQUEST_CODE_VK_CMD_DRAW_INDIRECT_COUNT 289
#define REQUEST_CODE_VK_CMD_DRAW_INDEXED_INDIRECT_COUNT 290
#define REQUEST_CODE_VK_CMD_BIND_TRANSFORM_FEEDBACK_BUFFERS_EXT 291
#define REQUEST_CODE_VK_CMD_BEGIN_TRANSFORM_FEEDBACK_EXT 292
#define REQUEST_CODE_VK_CMD_END_TRANSFORM_FEEDBACK_EXT 293
#define REQUEST_CODE_VK_CMD_BEGIN_QUERY_INDEXED_EXT 294
#define REQUEST_CODE_VK_CMD_END_QUERY_INDEXED_EXT 295
#define REQUEST_CODE_VK_CMD_DRAW_INDIRECT_BYTE_COUNT_EXT 296
#define REQUEST_CODE_VK_GET_BUFFER_OPAQUE_CAPTURE_ADDRESS 297
#define REQUEST_CODE_VK_GET_BUFFER_DEVICE_ADDRESS 298
#define REQUEST_CODE_VK_GET_DEVICE_MEMORY_OPAQUE_CAPTURE_ADDRESS 299
#define REQUEST_CODE_VK_CMD_SET_LINE_STIPPLE_KHR 300
#define REQUEST_CODE_VK_CMD_SET_CULL_MODE 301
#define REQUEST_CODE_VK_CMD_SET_FRONT_FACE 302
#define REQUEST_CODE_VK_CMD_SET_PRIMITIVE_TOPOLOGY 303
#define REQUEST_CODE_VK_CMD_SET_VIEWPORT_WITH_COUNT 304
#define REQUEST_CODE_VK_CMD_SET_SCISSOR_WITH_COUNT 305
#define REQUEST_CODE_VK_CMD_BIND_VERTEX_BUFFERS2 306
#define REQUEST_CODE_VK_CMD_SET_DEPTH_TEST_ENABLE 307
#define REQUEST_CODE_VK_CMD_SET_DEPTH_WRITE_ENABLE 308
#define REQUEST_CODE_VK_CMD_SET_DEPTH_COMPARE_OP 309
#define REQUEST_CODE_VK_CMD_SET_DEPTH_BOUNDS_TEST_ENABLE 310
#define REQUEST_CODE_VK_CMD_SET_STENCIL_TEST_ENABLE 311
#define REQUEST_CODE_VK_CMD_SET_STENCIL_OP 312
#define REQUEST_CODE_VK_CMD_SET_RASTERIZER_DISCARD_ENABLE 313
#define REQUEST_CODE_VK_CMD_SET_DEPTH_BIAS_ENABLE 314
#define REQUEST_CODE_VK_CMD_SET_PRIMITIVE_RESTART_ENABLE 315
#define REQUEST_CODE_VK_CMD_SET_TESSELLATION_DOMAIN_ORIGIN_EXT 316
#define REQUEST_CODE_VK_CMD_SET_DEPTH_CLAMP_ENABLE_EXT 317
#define REQUEST_CODE_VK_CMD_SET_POLYGON_MODE_EXT 318
#define REQUEST_CODE_VK_CMD_SET_RASTERIZATION_SAMPLES_EXT 319
#define REQUEST_CODE_VK_CMD_SET_SAMPLE_MASK_EXT 320
#define REQUEST_CODE_VK_CMD_SET_ALPHA_TO_COVERAGE_ENABLE_EXT 321
#define REQUEST_CODE_VK_CMD_SET_ALPHA_TO_ONE_ENABLE_EXT 322
#define REQUEST_CODE_VK_CMD_SET_LOGIC_OP_ENABLE_EXT 323
#define REQUEST_CODE_VK_CMD_SET_COLOR_BLEND_ENABLE_EXT 324
#define REQUEST_CODE_VK_CMD_SET_COLOR_BLEND_EQUATION_EXT 325
#define REQUEST_CODE_VK_CMD_SET_COLOR_WRITE_MASK_EXT 326
#define REQUEST_CODE_VK_CMD_SET_RASTERIZATION_STREAM_EXT 327
#define REQUEST_CODE_VK_CMD_SET_CONSERVATIVE_RASTERIZATION_MODE_EXT 328
#define REQUEST_CODE_VK_CMD_SET_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT 329
#define REQUEST_CODE_VK_CMD_SET_DEPTH_CLIP_ENABLE_EXT 330
#define REQUEST_CODE_VK_CMD_SET_SAMPLE_LOCATIONS_ENABLE_EXT 331
#define REQUEST_CODE_VK_CMD_SET_COLOR_BLEND_ADVANCED_EXT 332
#define REQUEST_CODE_VK_CMD_SET_PROVOKING_VERTEX_MODE_EXT 333
#define REQUEST_CODE_VK_CMD_SET_LINE_RASTERIZATION_MODE_EXT 334
#define REQUEST_CODE_VK_CMD_SET_LINE_STIPPLE_ENABLE_EXT 335
#define REQUEST_CODE_VK_CMD_SET_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT 336
#define REQUEST_CODE_VK_CMD_COPY_BUFFER2 337
#define REQUEST_CODE_VK_CMD_COPY_IMAGE2 338
#define REQUEST_CODE_VK_CMD_BLIT_IMAGE2 339
#define REQUEST_CODE_VK_CMD_COPY_BUFFER_TO_IMAGE2 340
#define REQUEST_CODE_VK_CMD_COPY_IMAGE_TO_BUFFER2 341
#define REQUEST_CODE_VK_CMD_RESOLVE_IMAGE2 342
#define REQUEST_CODE_VK_CMD_SET_COLOR_WRITE_ENABLE_EXT 343
#define REQUEST_CODE_VK_CMD_SET_EVENT2 344
#define REQUEST_CODE_VK_CMD_RESET_EVENT2 345
#define REQUEST_CODE_VK_CMD_WAIT_EVENTS2 346
#define REQUEST_CODE_VK_CMD_PIPELINE_BARRIER2 347
#define REQUEST_CODE_VK_QUEUE_SUBMIT2 348
#define REQUEST_CODE_VK_CMD_WRITE_TIMESTAMP2 349
#define REQUEST_CODE_VK_CMD_BEGIN_RENDERING 350
#define REQUEST_CODE_VK_CMD_END_RENDERING 351
#define REQUEST_CODE_VK_GET_SHADER_MODULE_IDENTIFIER_EXT 352
#define REQUEST_CODE_VK_GET_SHADER_MODULE_CREATE_INFO_IDENTIFIER_EXT 353

#if DEBUG_MODE
static const char* requestCodeStrings[] = {
    "vkCreateInstance",
    "vkDestroyInstance",
    "vkEnumeratePhysicalDevices",
    "vkGetPhysicalDeviceProperties",
    "vkGetPhysicalDeviceQueueFamilyProperties",
    "vkGetPhysicalDeviceMemoryProperties",
    "vkGetPhysicalDeviceFeatures",
    "vkGetPhysicalDeviceFormatProperties",
    "vkGetPhysicalDeviceImageFormatProperties",
    "vkCreateDevice",
    "vkDestroyDevice",
    "vkEnumerateInstanceVersion",
    "vkEnumerateInstanceExtensionProperties",
    "vkEnumerateDeviceExtensionProperties",
    "vkGetDeviceQueue",
    "vkQueueSubmit",
    "vkQueueWaitIdle",
    "vkDeviceWaitIdle",
    "vkAllocateMemory",
    "vkFreeMemory",
    "vkMapMemory",
    "vkUnmapMemory",
    "vkFlushMappedMemoryRanges",
    "vkInvalidateMappedMemoryRanges",
    "vkGetDeviceMemoryCommitment",
    "vkGetBufferMemoryRequirements",
    "vkBindBufferMemory",
    "vkGetImageMemoryRequirements",
    "vkBindImageMemory",
    "vkGetImageSparseMemoryRequirements",
    "vkGetPhysicalDeviceSparseImageFormatProperties",
    "vkQueueBindSparse",
    "vkCreateFence",
    "vkDestroyFence",
    "vkResetFences",
    "vkGetFenceStatus",
    "vkWaitForFences",
    "vkCreateSemaphore",
    "vkDestroySemaphore",
    "vkCreateEvent",
    "vkDestroyEvent",
    "vkGetEventStatus",
    "vkSetEvent",
    "vkResetEvent",
    "vkCreateQueryPool",
    "vkDestroyQueryPool",
    "vkGetQueryPoolResults",
    "vkResetQueryPool",
    "vkCreateBuffer",
    "vkDestroyBuffer",
    "vkCreateBufferView",
    "vkDestroyBufferView",
    "vkCreateImage",
    "vkDestroyImage",
    "vkGetImageSubresourceLayout",
    "vkCreateImageView",
    "vkDestroyImageView",
    "vkCreateShaderModule",
    "vkDestroyShaderModule",
    "vkCreatePipelineCache",
    "vkDestroyPipelineCache",
    "vkGetPipelineCacheData",
    "vkMergePipelineCaches",
    "vkCreateGraphicsPipelines",
    "vkCreateComputePipelines",
    "vkDestroyPipeline",
    "vkCreatePipelineLayout",
    "vkDestroyPipelineLayout",
    "vkCreateSampler",
    "vkDestroySampler",
    "vkCreateDescriptorSetLayout",
    "vkDestroyDescriptorSetLayout",
    "vkCreateDescriptorPool",
    "vkDestroyDescriptorPool",
    "vkResetDescriptorPool",
    "vkAllocateDescriptorSets",
    "vkFreeDescriptorSets",
    "vkUpdateDescriptorSets",
    "vkCreateFramebuffer",
    "vkDestroyFramebuffer",
    "vkCreateRenderPass",
    "vkDestroyRenderPass",
    "vkGetRenderAreaGranularity",
    "vkCreateCommandPool",
    "vkDestroyCommandPool",
    "vkResetCommandPool",
    "vkAllocateCommandBuffers",
    "vkFreeCommandBuffers",
    "vkBeginCommandBuffer",
    "vkEndCommandBuffer",
    "vkResetCommandBuffer",
    "vkCmdBindPipeline",
    "vkCmdSetViewport",
    "vkCmdSetScissor",
    "vkCmdSetLineWidth",
    "vkCmdSetDepthBias",
    "vkCmdSetBlendConstants",
    "vkCmdSetDepthBounds",
    "vkCmdSetStencilCompareMask",
    "vkCmdSetStencilWriteMask",
    "vkCmdSetStencilReference",
    "vkCmdBindDescriptorSets",
    "vkCmdBindIndexBuffer",
    "vkCmdBindVertexBuffers",
    "vkCmdDraw",
    "vkCmdDrawIndexed",
    "vkCmdDrawIndirect",
    "vkCmdDrawIndexedIndirect",
    "vkCmdDispatch",
    "vkCmdDispatchIndirect",
    "vkCmdCopyBuffer",
    "vkCmdCopyImage",
    "vkCmdBlitImage",
    "vkCmdCopyBufferToImage",
    "vkCmdCopyImageToBuffer",
    "vkCmdUpdateBuffer",
    "vkCmdFillBuffer",
    "vkCmdClearColorImage",
    "vkCmdClearDepthStencilImage",
    "vkCmdClearAttachments",
    "vkCmdResolveImage",
    "vkCmdSetEvent",
    "vkCmdResetEvent",
    "vkCmdWaitEvents",
    "vkCmdPipelineBarrier",
    "vkCmdBeginQuery",
    "vkCmdEndQuery",
    "vkCmdBeginConditionalRenderingEXT",
    "vkCmdEndConditionalRenderingEXT",
    "vkCmdResetQueryPool",
    "vkCmdWriteTimestamp",
    "vkCmdCopyQueryPoolResults",
    "vkCmdPushConstants",
    "vkCmdBeginRenderPass",
    "vkCmdNextSubpass",
    "vkCmdEndRenderPass",
    "vkCmdExecuteCommands",
    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
    "vkGetPhysicalDeviceSurfaceFormatsKHR",
    "vkGetPhysicalDeviceSurfacePresentModesKHR",
    "vkCreateSwapchainKHR",
    "vkDestroySwapchainKHR",
    "vkGetSwapchainImagesKHR",
    "vkAcquireNextImageKHR",
    "vkQueuePresentKHR",
    "vkGetPhysicalDeviceFeatures2",
    "vkGetPhysicalDeviceProperties2",
    "vkGetPhysicalDeviceFormatProperties2",
    "vkGetPhysicalDeviceImageFormatProperties2",
    "vkGetPhysicalDeviceQueueFamilyProperties2",
    "vkGetPhysicalDeviceMemoryProperties2",
    "vkGetPhysicalDeviceSparseImageFormatProperties2",
    "vkCmdPushDescriptorSetKHR",
    "vkTrimCommandPool",
    "vkGetPhysicalDeviceExternalBufferProperties",
    "vkGetMemoryFdKHR",
    "vkGetPhysicalDeviceExternalSemaphoreProperties",
    "vkGetSemaphoreFdKHR",
    "vkGetPhysicalDeviceExternalFenceProperties",
    "vkGetFenceFdKHR",
    "vkEnumeratePhysicalDeviceGroups",
    "vkGetDeviceGroupPeerMemoryFeatures",
    "vkBindBufferMemory2",
    "vkBindImageMemory2",
    "vkCmdSetDeviceMask",
    "vkAcquireNextImage2KHR",
    "vkCmdDispatchBase",
    "vkGetPhysicalDevicePresentRectanglesKHR",
    "vkCmdSetSampleLocationsEXT",
    "vkGetPhysicalDeviceMultisamplePropertiesEXT",
    "vkGetBufferMemoryRequirements2",
    "vkGetImageMemoryRequirements2",
    "vkGetImageSparseMemoryRequirements2",
    "vkGetDeviceBufferMemoryRequirements",
    "vkGetDeviceImageMemoryRequirements",
    "vkGetDeviceImageSparseMemoryRequirements",
    "vkCreateSamplerYcbcrConversion",
    "vkDestroySamplerYcbcrConversion",
    "vkGetDeviceQueue2",
    "vkGetDescriptorSetLayoutSupport",
    "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR",
    "vkGetCalibratedTimestampsKHR",
    "vkCreateRenderPass2",
    "vkCmdBeginRenderPass2",
    "vkCmdNextSubpass2",
    "vkCmdEndRenderPass2",
    "vkGetSemaphoreCounterValue",
    "vkWaitSemaphores",
    "vkSignalSemaphore",
    "vkCmdDrawIndirectCount",
    "vkCmdDrawIndexedIndirectCount",
    "vkCmdBindTransformFeedbackBuffersEXT",
    "vkCmdBeginTransformFeedbackEXT",
    "vkCmdEndTransformFeedbackEXT",
    "vkCmdBeginQueryIndexedEXT",
    "vkCmdEndQueryIndexedEXT",
    "vkCmdDrawIndirectByteCountEXT",
    "vkGetBufferOpaqueCaptureAddress",
    "vkGetBufferDeviceAddress",
    "vkGetDeviceMemoryOpaqueCaptureAddress",
    "vkCmdSetLineStippleKHR",
    "vkCmdSetCullMode",
    "vkCmdSetFrontFace",
    "vkCmdSetPrimitiveTopology",
    "vkCmdSetViewportWithCount",
    "vkCmdSetScissorWithCount",
    "vkCmdBindVertexBuffers2",
    "vkCmdSetDepthTestEnable",
    "vkCmdSetDepthWriteEnable",
    "vkCmdSetDepthCompareOp",
    "vkCmdSetDepthBoundsTestEnable",
    "vkCmdSetStencilTestEnable",
    "vkCmdSetStencilOp",
    "vkCmdSetRasterizerDiscardEnable",
    "vkCmdSetDepthBiasEnable",
    "vkCmdSetPrimitiveRestartEnable",
    "vkCmdSetTessellationDomainOriginEXT",
    "vkCmdSetDepthClampEnableEXT",
    "vkCmdSetPolygonModeEXT",
    "vkCmdSetRasterizationSamplesEXT",
    "vkCmdSetSampleMaskEXT",
    "vkCmdSetAlphaToCoverageEnableEXT",
    "vkCmdSetAlphaToOneEnableEXT",
    "vkCmdSetLogicOpEnableEXT",
    "vkCmdSetColorBlendEnableEXT",
    "vkCmdSetColorBlendEquationEXT",
    "vkCmdSetColorWriteMaskEXT",
    "vkCmdSetRasterizationStreamEXT",
    "vkCmdSetConservativeRasterizationModeEXT",
    "vkCmdSetExtraPrimitiveOverestimationSizeEXT",
    "vkCmdSetDepthClipEnableEXT",
    "vkCmdSetSampleLocationsEnableEXT",
    "vkCmdSetColorBlendAdvancedEXT",
    "vkCmdSetProvokingVertexModeEXT",
    "vkCmdSetLineRasterizationModeEXT",
    "vkCmdSetLineStippleEnableEXT",
    "vkCmdSetDepthClipNegativeOneToOneEXT",
    "vkCmdCopyBuffer2",
    "vkCmdCopyImage2",
    "vkCmdBlitImage2",
    "vkCmdCopyBufferToImage2",
    "vkCmdCopyImageToBuffer2",
    "vkCmdResolveImage2",
    "vkCmdSetColorWriteEnableEXT",
    "vkCmdSetEvent2",
    "vkCmdResetEvent2",
    "vkCmdWaitEvents2",
    "vkCmdPipelineBarrier2",
    "vkQueueSubmit2",
    "vkCmdWriteTimestamp2",
    "vkCmdBeginRendering",
    "vkCmdEndRendering",
    "vkGetShaderModuleIdentifierEXT",
    "vkGetShaderModuleCreateInfoIdentifierEXT",
};

static inline const char* requestCodeToString(short requestCode) {
    int index = requestCode - REQUEST_CODE_VK_CALL_START;
    return index >= 0 && index < REQUEST_CODE_VK_CALL_COUNT ? requestCodeStrings[index] : "Unknown";
}
#endif

#endif