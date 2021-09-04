#include <vk_textures.h>
#include <vk_initializers.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace vkutil
{
    bool LoadImageFromFile(VulkanEngine& engine, const char* file_name, AllocatedImage& out_image)
    {
        int texture_width;
        int texture_height;
        int texture_channels;

        // Load image from file
        stbi_uc* pixels = stbi_load(file_name, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
        if (!pixels)
        {
            std::cout << "Failed to load textures file: " << file_name << std::endl;
            return false;
        }

        // Make CPU buffer to store image data and copy
        void* pixel_ptr = pixels;
        VkDeviceSize image_size         = texture_width * texture_height * 4;
        VkFormat image_format           = VK_FORMAT_R8G8B8A8_SRGB;
        AllocatedBuffer staging_buffer  = engine.CreateBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        void* data;
        vmaMapMemory(engine._allocator, staging_buffer.allocation, &data);
        memcpy(data, pixel_ptr, static_cast<size_t>(image_size));
        vmaUnmapMemory(engine._allocator, staging_buffer.allocation);
        stbi_image_free(pixels);

        // Create VkImage instance with info
        VkExtent3D image_extent{};
        image_extent.width  = static_cast<uint32_t>(texture_width);
        image_extent.height = static_cast<uint32_t>(texture_height);
        image_extent.depth  = 1;

        VkImageCreateInfo   create_image_info = vkinit::ImageCreatInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, image_extent);
        AllocatedImage      new_image;

        VmaAllocationCreateInfo image_allocation_info{};
        image_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        vmaCreateImage(engine._allocator, &create_image_info, &image_allocation_info, &new_image.image, &new_image.allocation, nullptr);

        // Submit render call to engine
        engine.ImmediateSubmit([&](VkCommandBuffer command_buffer)
            {
                VkImageSubresourceRange image_resource_range{};
                image_resource_range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                image_resource_range.baseMipLevel   = 0;
                image_resource_range.levelCount     = 1;
                image_resource_range.baseArrayLayer = 0;
                image_resource_range.layerCount     = 1;

                // Pipeline barriers to control how GPU overlaps commands
                VkImageMemoryBarrier image_barrier_transfer{};
                image_barrier_transfer.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                image_barrier_transfer.pNext            = nullptr;
                image_barrier_transfer.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED;
                image_barrier_transfer.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                image_barrier_transfer.image            = new_image.image;
                image_barrier_transfer.subresourceRange = image_resource_range;
                image_barrier_transfer.srcAccessMask    = 0;
                image_barrier_transfer.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;

                vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_transfer);

                // Fill in pixel data for image
                VkBufferImageCopy image_copy_region{};
                image_copy_region.bufferOffset                      = 0;
                image_copy_region.bufferRowLength                   = 0;
                image_copy_region.bufferImageHeight                 = 0;
                image_copy_region.imageSubresource.aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
                image_copy_region.imageSubresource.mipLevel         = 0;
                image_copy_region.imageSubresource.baseArrayLayer   = 0;
                image_copy_region.imageSubresource.layerCount       = 1;
                image_copy_region.imageExtent                       = image_extent;

                vkCmdCopyBufferToImage(command_buffer, staging_buffer.buffer, new_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &image_copy_region);

                // change layout to be shader readable
                VkImageMemoryBarrier image_barrier_readable = image_barrier_transfer;
                image_barrier_readable.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                image_barrier_readable.newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                image_barrier_readable.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
                image_barrier_readable.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
                
                vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_barrier_readable);
            });

        engine._main_deletion_queue.PushFunction([=]()
            {
                std::cout << "Deleting image from inside LoadImageFromFile\n";
                vmaDestroyImage(engine._allocator, new_image.image, new_image.allocation);
            });

        vmaDestroyBuffer(engine._allocator, staging_buffer.buffer, staging_buffer.allocation);

        std::cout << "Texture loaded successfully " << file_name << "\n";

        out_image = new_image;
        return true;
    }
}
