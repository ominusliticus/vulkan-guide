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
}


