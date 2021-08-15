#include <vk_initializers.h>

namespace vkinit
{
	VkCommandPoolCreateInfo	CommandPoolCreateInfo(uint32_t queue_family_index, VkCommandPoolCreateFlags flags /* = 0 */)
	{
		// Create command pools for submitting to graphics queue
		VkCommandPoolCreateInfo command_pool_info = {};	// Very important to initialize with empty initializer_list
		command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_info.pNext = nullptr;

		// command pool submits to graphics queue
		command_pool_info.queueFamilyIndex = queue_family_index;
		command_pool_info.flags = flags;
		return command_pool_info;
	}

	VkCommandBufferAllocateInfo	CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count /* = 1 */, VkCommandBufferLevel level /* = VK_COMMAND_BUFFER_LEVEL_PRIMARY*/)
	{
		// Create command buffers used for rendering
		VkCommandBufferAllocateInfo command_buffer_info = {};	// Very important to initialize with empty initializer_list
		command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_info.pNext = nullptr;

		command_buffer_info.commandPool = pool;							// command buffer will be made from our command pool
		command_buffer_info.commandBufferCount = count;					// will allocate one command buffer (could be more)
		command_buffer_info.level = level;
		return command_buffer_info;
	}

	VkRenderPassCreateInfo RenderPassCreateInfo(VkAttachmentDescription* color_attachment, uint32_t attachment_count, VkSubpassDescription* subpasses, uint32_t subpass_count)
	{
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = attachment_count;
		render_pass_info.pAttachments = color_attachment;
		render_pass_info.subpassCount = subpass_count;
		render_pass_info.pSubpasses = subpasses;
		return render_pass_info;
	}
}


