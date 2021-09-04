#include <vk_initializers.h>

namespace vkinit
{
    VkCommandPoolCreateInfo	CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags /* = 0 */)
    {
        // Create command pools for submitting to graphics queue
        VkCommandPoolCreateInfo command_pool_info{};	// Very important to initialize with empty initializer_list
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.pNext = nullptr;

        // command pool submits to graphics queue
        command_pool_info.queueFamilyIndex  = queue_family_index;
        command_pool_info.flags             = flags;
        return command_pool_info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkCommandBufferAllocateInfo	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count /* = 1 */, VkCommandBufferLevel level /* = VK_COMMAND_BUFFER_LEVEL_PRIMARY*/)
    {
        // Create command buffers used for rendering
        VkCommandBufferAllocateInfo command_buffer_info{};	// Very important to initialize with empty initializer_list
        command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_info.pNext = nullptr;

        command_buffer_info.commandPool         = pool;						// command buffer will be made from our command pool
        command_buffer_info.commandBufferCount  = count;					// will allocate one command buffer (could be more)
        command_buffer_info.level               = level;
        return command_buffer_info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferResetFlags flags, VkCommandBufferInheritanceInfo* inheritance_info /* = nullptr */)
    {
        VkCommandBufferBeginInfo info{};
        info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.pNext              = nullptr;
        info.flags              = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        info.pInheritanceInfo   = nullptr;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkRenderPassCreateInfo RenderPassCreateInfo(VkAttachmentDescription* color_attachment, uint32_t attachment_count, VkSubpassDescription* subpasses, uint32_t subpass_count)
    {
        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount    = attachment_count;
        render_pass_info.pAttachments       = color_attachment;
        render_pass_info.subpassCount       = subpass_count;
        render_pass_info.pSubpasses         = subpasses;
        return render_pass_info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkRenderPassBeginInfo RenderPassBeginInfo(VkRenderPass render_pass, VkExtent2D extent, VkFramebuffer framebuffer)
    {
        VkRenderPassBeginInfo info{};
        info.sType				    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.pNext				    = nullptr;
        info.renderPass			    = render_pass;
        info.renderArea.offset.x    = 0;
        info.renderArea.offset.y	= 0;
        info.renderArea.extent	    = extent;
        info.framebuffer			= framebuffer;
        info.clearValueCount		= 1;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkSubmitInfo SubmitInfo(VkCommandBuffer* command_buffer)
    {
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.pNext                  = nullptr;
        info.pWaitDstStageMask      = 0;
        info.waitSemaphoreCount     = 0;
        info.pWaitSemaphores        = nullptr;
        info.signalSemaphoreCount   = 0;
        info.pSignalSemaphores      = nullptr;
        info.commandBufferCount     = 1;
        info.pCommandBuffers        = command_buffer;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineShaderStageCreateInfo	PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shader_module)
    {
        VkPipelineShaderStageCreateInfo info{};
        info.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.pNext  = nullptr;
        info.stage  = stage;
        info.module = shader_module;
        info.pName  = "main";
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineVertexInputStateCreateInfo	VertexInputStateCreateInfo()
    {
        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.pNext = nullptr;
        
        // no vertex bindings and attributes
        info.vertexBindingDescriptionCount   = 0;
        info.vertexAttributeDescriptionCount = 0;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineInputAssemblyStateCreateInfo	InputAssemblyCreateInfo(VkPrimitiveTopology topology)
    {
        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.pNext                  = 0;
        info.topology               = topology;
        info.primitiveRestartEnable = VK_FALSE;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineRasterizationStateCreateInfo	RasterizationStateCreateInfo(VkPolygonMode polygon_mode)
    {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.pNext                      = nullptr;
        info.depthClampEnable           = VK_FALSE;
        info.rasterizerDiscardEnable    = VK_FALSE;                 // Discards all primitives before rasterization when true
        info.polygonMode                = polygon_mode;
        info.lineWidth                  = 1.0f;
        info.cullMode                   = VK_CULL_MODE_NONE;        // No backface cull
        info.frontFace                  = VK_FRONT_FACE_CLOCKWISE;
        info.depthBiasEnable            = VK_FALSE;                 // No depth bias
        info.depthBiasConstantFactor    = 0.0f;
        info.depthBiasClamp             = 0.0f;
        info.depthBiasSlopeFactor       = 0.0f;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineColorBlendAttachmentState		ColorBlendAttachmentState()
    {
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask   = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable      = VK_FALSE;
        return color_blend_attachment;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineMultisampleStateCreateInfo	MultisamplingStateCreateInfo()
    {
        VkPipelineMultisampleStateCreateInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; 
        info.pNext                  = 0;
        info.sampleShadingEnable    = VK_FALSE;
        info.rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT;  // One sample per pixel
        info.minSampleShading       = 1.0f;
        info.pSampleMask            = nullptr;
        info.alphaToCoverageEnable  = VK_FALSE;
        info.alphaToOneEnable       = VK_FALSE;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo()
    {
        VkPipelineLayoutCreateInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        info.pNext                  = nullptr;
        info.flags                  = 0;
        info.setLayoutCount         = 0;
        info.pSetLayouts            = nullptr;
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges    = nullptr;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkImageCreateInfo ImageCreatInfo(VkFormat format, VkImageUsageFlags usage_flags, VkExtent3D extent)
    {
        VkImageCreateInfo info{};
        info.sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext          = nullptr;
        info.imageType      = VK_IMAGE_TYPE_2D;
        info.format         = format;
        info.extent         = extent;
        info.mipLevels      = 1;
        info.arrayLayers    = 1;
        info.samples        = VK_SAMPLE_COUNT_1_BIT;
        info.tiling         = VK_IMAGE_TILING_OPTIMAL;
        info.usage          = usage_flags;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkImageViewCreateInfo ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspect_flags)
    {
        VkImageViewCreateInfo info{};
        info.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext                              = nullptr;
        info.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
        info.image                              = image;
        info.format                             = format;
        info.subresourceRange.baseMipLevel      = 0;
        info.subresourceRange.levelCount        = 1;
        info.subresourceRange.baseArrayLayer    = 0;
        info.subresourceRange.layerCount        = 1;
        info.subresourceRange.aspectMask        = aspect_flags;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool b_depth_test, bool b_depth_write, VkCompareOp compare_op)
    {
        VkPipelineDepthStencilStateCreateInfo info{};
        info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.pNext                  = nullptr;
        info.depthTestEnable        = b_depth_test  ?    VK_TRUE : VK_FALSE;
        info.depthWriteEnable       = b_depth_write ?    VK_TRUE : VK_FALSE;
        info.depthCompareOp         = b_depth_test  ? compare_op : VK_COMPARE_OP_ALWAYS;
        info.depthBoundsTestEnable  = VK_FALSE;
        info.minDepthBounds         = 0.0f;
        info.maxDepthBounds         = 1.0f;
        info.stencilTestEnable      = VK_FALSE;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags /* = 0 */)
    {
        VkSemaphoreCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags /* = 0 */)
    {
        VkFenceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = flags;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type, VkShaderStageFlags stage_flags, uint32_t binding)
    {
        VkDescriptorSetLayoutBinding set_binding{};
        set_binding.binding             = binding;
        set_binding.descriptorCount     = 1;
        set_binding.descriptorType      = type;
        set_binding.pImmutableSamplers  = nullptr;
        set_binding.stageFlags          = stage_flags;
        return set_binding;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorType type, VkDescriptorSet descriptor_set, VkDescriptorBufferInfo* buffer_info, uint32_t binding)
    {
        VkWriteDescriptorSet set{};
        set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.pNext           = nullptr;
        set.dstBinding      = binding;
        set.dstSet          = descriptor_set;
        set.descriptorCount = 1;
        set.descriptorType  = type;
        set.pBufferInfo     = buffer_info;
        return set;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorType type, VkDescriptorSet descriptor_set, VkDescriptorImageInfo* image_info, uint32_t binding)
    {
        VkWriteDescriptorSet set{};
        set.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        set.pNext           = nullptr;
        set.dstBinding      = binding;
        set.dstSet          = descriptor_set;
        set.descriptorCount = 1;
        set.descriptorType  = type;
        set.pImageInfo      = image_info;
        return set;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range)
    {
        VkDescriptorBufferInfo info{};
        info.buffer = buffer;
        info.offset = offset;
        info.range  = range;
        return info;
    }

    // .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

    VkSamplerCreateInfo SamplerCreateInfo(VkFilter filters, VkSamplerAddressMode sampler_address_mode /* = VK_SAMPLER_ADDRESS_MODE_REPEAT */)
    {
        VkSamplerCreateInfo info{};
        info.sType          = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.pNext          = nullptr;
        info.magFilter      = filters;
        info.minFilter      = filters;
        info.addressModeU   = sampler_address_mode;
        info.addressModeV   = sampler_address_mode;
        info.addressModeW   = sampler_address_mode;
        return info;
    }
}


