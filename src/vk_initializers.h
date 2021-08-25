// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

namespace vkinit 
{
    VkCommandPoolCreateInfo		CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VkRenderPassCreateInfo		RenderPassCreateInfo(VkAttachmentDescription* color_attachment, uint32_t attachment_count, VkSubpassDescription* subpasses, uint32_t subpass_count);
    VkRenderPassBeginInfo       RenderPassBeginInfo(VkRenderPass render_pass, VkExtent2D extent, VkFramebuffer framebuffer);

    // Pipeline struct initializations
    VkPipelineShaderStageCreateInfo			PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module);
    VkPipelineVertexInputStateCreateInfo	VertexInputStateCreateInfo();
    VkPipelineInputAssemblyStateCreateInfo	InputAssemblyCreateInfo(VkPrimitiveTopology topology);
    VkPipelineRasterizationStateCreateInfo	RasterizationStateCreateInfo(VkPolygonMode polygon_mode);
    VkPipelineColorBlendAttachmentState		ColorBlendAttachmentState();
    VkPipelineMultisampleStateCreateInfo	MultisamplingStateCreateInfo();

    // Pipeline layouts need to be created separately
    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

    // Depth buffer initializations
    VkImageCreateInfo                       ImageCreatInfo(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent);
    VkImageViewCreateInfo                   ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags);
    VkPipelineDepthStencilStateCreateInfo   DepthStencilCreateInfo(bool b_depth_test, bool b_depth_write, VkCompareOp compare_op);

    // Initialize synchronoization structures
    VkSemaphoreCreateInfo   SemaphoreCreateInfo(VkSemaphoreCreateFlags flags=0);
    VkFenceCreateInfo       FenceCreateInfo(VkFenceCreateFlags flags);
}

