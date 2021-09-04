// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>

namespace vkinit
{
    VkCommandPoolCreateInfo		CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags = 0);
    VkCommandBufferAllocateInfo	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    VkCommandBufferBeginInfo    CommandBufferBeginInfo(VkCommandBufferResetFlags flags, VkCommandBufferInheritanceInfo* inheritance_info = nullptr);
    VkRenderPassCreateInfo		RenderPassCreateInfo(VkAttachmentDescription* color_attachment, uint32_t attachment_count, VkSubpassDescription* subpasses, uint32_t subpass_count);
    VkRenderPassBeginInfo       RenderPassBeginInfo(VkRenderPass render_pass, VkExtent2D extent, VkFramebuffer framebuffer);
    VkSubmitInfo                SubmitInfo(VkCommandBuffer* command_buffer);

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
    VkSemaphoreCreateInfo   SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
    VkFenceCreateInfo       FenceCreateInfo(VkFenceCreateFlags flags = 0);

    // Descriptor set initializations
    VkDescriptorSetLayoutBinding    DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding);
    VkWriteDescriptorSet            WriteDescriptorSet(VkDescriptorType type, VkDescriptorSet descriptor_set, VkDescriptorBufferInfo* buffer_info, uint32_t binding);
    VkWriteDescriptorSet            WriteDescriptorSet(VkDescriptorType type, VkDescriptorSet descriptor_set, VkDescriptorImageInfo* image_info, uint32_t binding);
    VkDescriptorBufferInfo          DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);

    // Image samplers
    VkSamplerCreateInfo SamplerCreateInfo(VkFilter filters, VkSamplerAddressMode sampler_address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

