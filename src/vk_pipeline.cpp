
#include <iostream>

#include "vk_pipeline.h"

#include <vk_types.h>
#include <vk_initializers.h>

// bootstrap library
#include <VkBootstrap.h>

VkPipeline PipelineBuilder::BuildPipeline(VkDevice device, VkRenderPass pass)
{
    VkPipelineViewportStateCreateInfo viewport_info{};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.pNext = nullptr;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports = &_viewport;
    viewport_info.scissorCount = 1;
    viewport_info.pScissors = &_scissor;

    VkPipelineColorBlendStateCreateInfo color_blend_info{};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.pNext = nullptr;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &_color_blend_attachement;

    // Build actual graphics pipeline
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;
    pipeline_info.stageCount = _shader_stages_infos.size();
    pipeline_info.pStages = _shader_stages_infos.data();
    pipeline_info.pVertexInputState = &_vertex_input_info;
    pipeline_info.pInputAssemblyState = &_input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &_rasterizer_info;
    pipeline_info.pMultisampleState = &_multisampling_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.layout = _pipeline_layout;
    pipeline_info.renderPass = pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline new_pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &new_pipeline) != VK_SUCCESS)
    {
        std::cout << "Failed to create pipeline\n";
        return VK_NULL_HANDLE;
    }
    else return new_pipeline;
}