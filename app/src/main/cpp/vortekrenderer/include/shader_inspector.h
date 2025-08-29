#ifndef VORTEK_SHADER_INSPECTOR_H
#define VORTEK_SHADER_INSPECTOR_H

#include "vortek.h"

typedef struct ShaderModule {
    VkShaderModule module;
    uint32_t* code;
    size_t codeSize;
} ShaderModule;

typedef struct ShaderInspector {
    bool checkClipDistance;       /* workaround for vertex explosion on Mali devices */
    bool convertFormatScaled;     /* workaround for t-pose issue on D3D9 games */
    bool removeImageBoundCheck;   /* workaround for black screen on Mali devices */
    bool removePointSizeExport;   /* workaround for graphical glitches on DXVK > 2.3.1 */
    bool checkInOutVariablesSize; /* workaround for freeze issues on DXVK > 2.3.1 */
} ShaderInspector;

extern ShaderInspector* ShaderInspector_create(VkContext* context, VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* supportedFeatures);
extern VkResult ShaderInspector_inspectShaderStages(ShaderInspector* shaderInspector, VkDevice device, VkPipelineShaderStageCreateInfo* stageInfos, uint32_t stageCount, const VkPipelineVertexInputStateCreateInfo* vertexInputState);
extern VkResult ShaderInspector_createModule(ShaderInspector* shaderInspector, VkDevice device, const uint32_t* code, size_t codeSize, ShaderModule** ppModule);
extern bool isFormatScaled(VkFormat format);

#endif