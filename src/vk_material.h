#pragma once

#include <vk_types.h>

struct Material
{
    VkPipeline          pipeline;
    VkPipelineLayout    pipeline_layout;

    VkDescriptorSet texture_descriptor_set{ VK_NULL_HANDLE };
};

