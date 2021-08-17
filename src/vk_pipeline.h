#pragma once


#include <vk_types.h>
#include <vk_initializers.h>
#include <vector>

// Pipeline class to contain intricacies of shader workflow
class PipelineBuilder
{
public:
    std::vector<VkPipelineShaderStageCreateInfo>	_shader_stages_infos;
    VkPipelineVertexInputStateCreateInfo			_vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo			_input_assembly_info;
    VkViewport										_viewport;
    VkRect2D										_scissor;
    VkPipelineRasterizationStateCreateInfo			_rasterizer_info;
    VkPipelineColorBlendAttachmentState				_color_blend_attachement;
    VkPipelineMultisampleStateCreateInfo			_multisampling_info;
    VkPipelineLayout								_pipeline_layout;

    VkPipeline BuildPipeline(VkDevice device, VkRenderPass pass);
};
