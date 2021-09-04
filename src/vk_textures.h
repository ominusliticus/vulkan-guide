#pragma once

#include <vk_types.h>
#include <vk_engine.h>


namespace vkutil
{
    bool LoadImageFromFile(VulkanEngine& engine, const char* file_name, AllocatedImage& out_image);
}