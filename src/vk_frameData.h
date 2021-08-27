#pragma once

#include <vk_types.h>
#include <vk_camera.h>

struct FrameData
{
    VkSemaphore present_semaphore;
    VkSemaphore render_semaphore;
    VkFence     render_fence;

    VkCommandPool   command_pool;
    VkCommandBuffer command_buffer;

    AllocatedBuffer camera_buffer;
    AllocatedBuffer object_buffer;

    VkDescriptorSet global_descriptor_set;
    VkDescriptorSet object_descriptor_set;
};