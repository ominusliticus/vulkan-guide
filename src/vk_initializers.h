// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

namespace vkinit 
{
    VkCommandPoolCreateInfo		CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VkRenderPassCreateInfo		RenderPassCreateInfo(VkAttachmentDescription* color_attachment, uint32_t attachment_count, VkSubpassDescription* subpasses, uint32_t subpass_count);

    // Pipeline struct initializations
    VkPipelineShaderStageCreateInfo			PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module);
    VkPipelineVertexInputStateCreateInfo	VertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo	InputAssemblyCreateInfo(VkPrimitiveTopology topology);
    VkPipelineRasterizationStateCreateInfo	RasterizationStateCreateInfo(VkPolygonMode polygon_mode);
    VkPipelineColorBlendAttachmentState		ColorBlendAttachmentState();
    VkPipelineMultisampleStateCreateInfo	MultisamplingStateCreateInfo();

    // Pipeline layouts need to be created separately
    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();
}

