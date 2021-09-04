
#include <iostream>

// macro that outputs any errors when called
#define VK_CHECK(x)															\
    do																		\
    {																		\
        VkResult err = x;													\
        if (err)															\
        {																	\
            std::cout << "Detected Vulkan error: " << err << std::endl;		\
        }																	\
    } while (0)																\

#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

#include <vk_types.h>
#include <vk_initializers.h>
#include <vk_textures.h>

// bootstrap library
#include <VkBootstrap.h>

// STL
#include <fstream>

// For VMA
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

// GLM Librbaries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void VulkanEngine::Init()
{
    // Initialize SDL and create a window with it
    SDL_Init(SDL_INIT_VIDEO);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

    // Create window 
    _window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _window_extent.width, _window_extent.height, window_flags);


    InitVulkan();				// load Vulkan core structures
    InitSwapchain();			// create swapchain
    InitCommands();				// create command pool(s) and command buffers
    InitDefaultRenderPass();	// Self evident name...no more commenting from here
    InitFrameBuffers();
    InitSyncStructure();		// Synchronozation for GPU: fences and semaphores
    InitCamera();
    InitDescriptors();
    InitPipeLines();
    LoadMeshes();
    LoadImages();
    InitScene();

    // Assuming everything initialized error free 
    _isInitialized = true;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::Cleanup()
{	
    // Since Vulkan API is written in C, everything needs to be cleaned up manually
    if (_isInitialized) 
    {	
        vkDeviceWaitIdle(_device);      // Wait for GPU to finish 
        _main_deletion_queue.Flush();   // Destroy Vulkan structs

        // destroy device, surface and the rest... in reverse order of initialization
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
        vkDestroyInstance(_instance, nullptr);
        SDL_DestroyWindow(_window);
    }
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::Draw()
{
    // 1 second in nano-seconds to avoid typing it out over and over again
    constexpr uint64_t one_second = 100000000;

    // Wait for GPU to finish rendering last frame, and time out for 1 sec
    VK_CHECK(vkWaitForFences(_device, 1, &GetCurrentFrame().render_fence, true, one_second));
    VK_CHECK(vkResetFences(_device, 1, &GetCurrentFrame().render_fence));

    // Request image from swap chain
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, one_second, GetCurrentFrame().present_semaphore, nullptr, &swapchain_image_index));

    // Reset command buffer for new render pass
    VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().command_buffer, 0));

    // Creating Begin Command Buffer instance
    VkCommandBufferBeginInfo command_buffer_begin_info = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VK_CHECK(vkBeginCommandBuffer(GetCurrentFrame().command_buffer, &command_buffer_begin_info));

    // Render pass that flashes a clear-color over time
    VkClearValue clear_value;
    float flash = std::abs(std::sin(_frame_number / 60.0f));
    clear_value.color = { { 0.0f, 0.0f, flash, 1.0f } };

    VkClearValue depth_clear_value;
    depth_clear_value.depthStencil.depth = 1.0f;

    // Combine clear colors
    VkClearValue clear_values[2] = { clear_value, depth_clear_value };
    // Start main render pass
    VkRenderPassBeginInfo render_pass_begin_info = vkinit::RenderPassBeginInfo(_render_pass, _window_extent, _frame_buffers[swapchain_image_index]);
    render_pass_begin_info.clearValueCount  = 2;
    render_pass_begin_info.pClearValues     = clear_values;    

    // Begin rendering pass
    vkCmdBeginRenderPass(GetCurrentFrame().command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    DrawObjects(GetCurrentFrame().command_buffer, _renderables.data(), _renderables.size());

    // End rendering pass
    vkCmdEndRenderPass(GetCurrentFrame().command_buffer);
    VK_CHECK(vkEndCommandBuffer(GetCurrentFrame().command_buffer));

    // Prepare to send to queue. Wait for _present_semaphore to finish before starting _render_semaphore
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    submit_info.pWaitDstStageMask	= &wait_stage;
    submit_info.waitSemaphoreCount	= 1;
    submit_info.pWaitSemaphores		= &GetCurrentFrame().present_semaphore;
    
    submit_info.signalSemaphoreCount	= 1;
    submit_info.pSignalSemaphores		= &GetCurrentFrame().render_semaphore;

    submit_info.commandBufferCount	= 1;
    submit_info.pCommandBuffers		= &GetCurrentFrame().command_buffer;

    // Submit command command buffers to queue and execute
    // _render_fence will now block until the graphics command is done
    VK_CHECK(vkQueueSubmit(_graphics_queue, 1, &submit_info, GetCurrentFrame().render_fence));

    // Display image to screen: wait until _render_semaphore is done 
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;

    present_info.swapchainCount		= 1;
    present_info.pSwapchains		= &_swapchain;
    present_info.waitSemaphoreCount	= 1;
    present_info.pWaitSemaphores	= &GetCurrentFrame().render_semaphore;
    present_info.pImageIndices		= &swapchain_image_index;

    VK_CHECK(vkQueuePresentKHR(_graphics_queue, &present_info));

    // increment frame count
    _frame_number++;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::Run()
{
    SDL_Event e;
    bool bQuit = false;

    //main loop
    while (!bQuit)
    {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            //close the window when user alt-f4s or clicks the X button			
            if (e.type == SDL_QUIT) bQuit = true;
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_SPACE)
                {
                    _selected_shader = (_selected_shader + 1) % 2;
                }

                // TODO: add 3D camera support
                // WASD implementation
                float speed = 0.1f;
                if (e.key.keysym.sym == SDLK_a)
                {
                    _camera.ProcessInput(CameraDirection::LEFT, speed);
                }
                if (e.key.keysym.sym == SDLK_d)
                {
                    _camera.ProcessInput(CameraDirection::RIGHT, speed);
                }
                if (e.key.keysym.sym == SDLK_s)
                {
                    _camera.ProcessInput(CameraDirection::BACKWARD, speed);
                }
                if (e.key.keysym.sym == SDLK_w)
                {
                    _camera.ProcessInput(CameraDirection::FORWARD, speed);
                }

                // Rest camera
                if (e.key.keysym.sym == SDLK_SPACE)
                    InitCamera();

                //if (e.key.keysym.sym == SDLK_a)
                //{
                //    _camera.ProcessInput(CameraDirection::LEFT, speed);
                //}
                //if (e.key.keysym.sym == SDLK_d)
                //{
                //    _camera.ProcessInput(CameraDirection::RIGHT, speed);
                //}
                //if (e.key.keysym.sym == SDLK_s)
                //{
                //    _camera.ProcessInput(CameraDirection::BACKWARD, speed);
                //}
                //if (e.key.keysym.sym == SDLK_w)
                //{
                //    _camera.ProcessInput(CameraDirection::FORWARD, speed);
                //}
            }
        }

        int x, y;
        SDL_PumpEvents();
        uint32_t button = SDL_GetMouseState(&x, &y);
        //std::cout << "Mouse is at: ( " << x << " , " << y << " )\n";
        _camera.UpdateRotation(x, y, _window_extent);

        Draw();
    }
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitVulkan()
{
    vkb::InstanceBuilder builder;
     
    // Create Vulkan instance with basic debug features
    auto inst_ret = builder.set_app_name("Example Vulkan Application")
        .request_validation_layers(true)
        .require_api_version(1, 1, 0)
        .use_default_debug_messenger()
        .build();

    // Create and store instance
    vkb::Instance vkb_inst = inst_ret.value();
    _instance			= vkb_inst.instance;
    _debug_messenger	= vkb_inst.debug_messenger;


    // Get surface of window created with SDL
    SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

    // Use vkb to select GPU
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physical_device = selector
        .set_minimum_version(1, 1)
        .set_surface(_surface)
        .select()
        .value();

    // Create final Vulkan device
    vkb::DeviceBuilder device_builder{ physical_device };
    vkb::Device vkb_device = device_builder.build().value();

    _device    = vkb_device.device;
    _chosen_gpu = physical_device.physical_device;

    // use vkb to get graphics queue(s)
    _graphics_queue			= vkb_device.get_queue(vkb::QueueType::graphics).value();
    _graphics_queue_family	= vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize allocation buffers
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.physicalDevice   = _chosen_gpu;
    allocator_info.device           = _device;
    allocator_info.instance         = _instance;
    vmaCreateAllocator(&allocator_info, &_allocator);

    _main_deletion_queue.PushFunction([&]() {vmaDestroyAllocator(_allocator); });

    // Depth buffer components
    VkExtent3D depth_image_extent{ _window_extent.width, _window_extent.height, 1 };

    // hardcoding depth format to be 32 bit float;
    _depth_format = VK_FORMAT_D32_SFLOAT;   

    VkImageCreateInfo depth_image_info = vkinit::ImageCreatInfo(_depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

    // Allocate GPU memory for depth image
    VmaAllocationCreateInfo depth_image_allocation_info{};
    depth_image_allocation_info.usage           = VMA_MEMORY_USAGE_GPU_ONLY;
    depth_image_allocation_info.requiredFlags   = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vmaCreateImage(_allocator, &depth_image_info, &depth_image_allocation_info, &_depth_image.image, &_depth_image.allocation, nullptr);

    // build image-view from the depth image
    VkImageViewCreateInfo depth_image_view_info = vkinit::ImageViewCreateInfo(_depth_format, _depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(_device, &depth_image_view_info, nullptr, &_depth_image_view));

    _main_deletion_queue.PushFunction([=]()
        {
            std::cout << "Destroying depth image in InitVulkan()\n";
            vkDestroyImageView(_device, _depth_image_view, nullptr);
            vmaDestroyImage(_allocator, _depth_image.image, _depth_image.allocation);
        });

    vkGetPhysicalDeviceProperties(_chosen_gpu, &_gpu_properties);
    std::cout << "The GPU has a minimum buffer alignement of " << _gpu_properties.limits.minUniformBufferOffsetAlignment << std::endl;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitSwapchain()
{
    vkb::SwapchainBuilder swapchain_builder{ _chosen_gpu, _device, _surface };
    vkb::Swapchain vkb_swapchain = swapchain_builder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) // use vsync present mode??
        .set_desired_extent(_window_extent.width, _window_extent.height)
        .build()
        .value();

    // store swap chain
    _swapchain					= vkb_swapchain.swapchain;
    _swapchain_images			= vkb_swapchain.get_images().value();
    _swapchain_image_views		= vkb_swapchain.get_image_views().value();
    _swapchain_imagine_format	= vkb_swapchain.image_format;

    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Delete swapchain in InitSwapchain()\n";
            vkDestroySwapchainKHR(_device, _swapchain, nullptr); 
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitCommands()
{
    // Create command pool and get graphics queue
    VkCommandPoolCreateInfo command_pool_info = vkinit::CommandPoolCreateInfo(_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateCommandPool(_device, &command_pool_info, nullptr, &_frame_data[i].command_pool));
    
        // Create command buffer
        VkCommandBufferAllocateInfo command_buffer_info = vkinit::CommandBufferAllocateInfo(_frame_data[i].command_pool, 1);
        VK_CHECK(vkAllocateCommandBuffers(_device, &command_buffer_info, &_frame_data[i].command_buffer));

        _main_deletion_queue.PushFunction([=]() {vkDestroyCommandPool(_device, _frame_data[i].command_pool, nullptr); });
    }

    VkCommandPoolCreateInfo upload_context_command_pool_info = vkinit::CommandPoolCreateInfo(_graphics_queue_family);
    VK_CHECK(vkCreateCommandPool(_device, &upload_context_command_pool_info, nullptr, &_upload_context.command_pool));
    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Deleting command bool from InitCommands()\n";
            vkDestroyCommandPool(_device, _upload_context.command_pool, nullptr); 
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitDefaultRenderPass()
{
    // color attachment for render pass
    VkAttachmentDescription color_attachment{};	
    color_attachment.format			= _swapchain_imagine_format;		// set formate to that of swapchain
    color_attachment.samples		= VK_SAMPLE_COUNT_1_BIT;			// what is MSAA?
    color_attachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;		// clear when attachment is loaded
    color_attachment.storeOp		= VK_ATTACHMENT_STORE_OP_STORE;		// keep attachment stored when render pass ends
    color_attachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// don't care about stencils
    color_attachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;	// don't care about stencils
    color_attachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;		// for not we don't care
    color_attachment.finalLayout	= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// image has to be ready for display

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;										// attachment number indexed by render pass itself
    color_attachment_ref.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    _main_deletion_queue.PushFunction([=]() {vkDestroyRenderPass(_device, _render_pass, nullptr); });

    // Depth buffer code
    VkAttachmentDescription depth_attachment{};
    depth_attachment.flags          = 0;
    depth_attachment.format         = _depth_format;
    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount	= 1;
    subpass.pColorAttachments		= &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    // Image life so far: 
    // UNDEFINED -> RenderpPass begins 
    //				-> Subpass 0 begins (Transition to Attachment Optimal)
    //				-> Subpass 0 renders
    //				-> Subpass 0 ends		
    //			 -> RenderPass ends (Transition to Present Source)

    VkAttachmentDescription attachments[2]{ color_attachment, depth_attachment };
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount    = 2;
    render_pass_info.pAttachments       = attachments; 
    render_pass_info.subpassCount       = 1;
    render_pass_info.pSubpasses         = &subpass;

    VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_render_pass));
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitFrameBuffers()
{
    VkFramebufferCreateInfo frame_buffer_info{};
    frame_buffer_info.sType             = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.pNext             = nullptr;
    frame_buffer_info.renderPass        = _render_pass;
    frame_buffer_info.attachmentCount   = 1;
    frame_buffer_info.width             = _window_extent.width;
    frame_buffer_info.height            = _window_extent.height;
    frame_buffer_info.layers            = 1;

    uint32_t swapchain_image_count = _swapchain_images.size();
    _frame_buffers = std::vector<VkFramebuffer>(swapchain_image_count);

    // create frame buffers for each of the swapchain images
    for (int i = 0; i < swapchain_image_count; i++)
    {
        VkImageView image_view_attachments[2]{ _swapchain_image_views[i], _depth_image_view };

        frame_buffer_info.pAttachments      = image_view_attachments;
        frame_buffer_info.attachmentCount   = 2;

        VK_CHECK(vkCreateFramebuffer(_device, &frame_buffer_info, nullptr, &_frame_buffers[i]));
        
        _main_deletion_queue.PushFunction([=]() 
            {
                std::cout << "Deleting frame buffers from InitFrameBuffers()\n";
                vkDestroyFramebuffer(_device, _frame_buffers[i], nullptr); 
                vkDestroyImageView(_device, _swapchain_image_views[i], nullptr);
            });
    }

}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitSyncStructure()
{
    // Create fence
    VkFenceCreateInfo       fence_info      = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo   semaphore_info  = vkinit::SemaphoreCreateInfo();


    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_frame_data[i].render_fence));

        _main_deletion_queue.PushFunction([=]() 
            {
                std::cout << "Deleting rendering fences from InitSyncStructure()\n";
                vkDestroyFence(_device, _frame_data[i].render_fence, nullptr); 
            });


        VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frame_data[i].present_semaphore));
        VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_frame_data[i].render_semaphore));

        _main_deletion_queue.PushFunction([=]
            {
                std::cout << "Deleting rendering semaphores from InitSyncStructure()\n";
                vkDestroySemaphore(_device, _frame_data[i].present_semaphore, nullptr);
                vkDestroySemaphore(_device, _frame_data[i].render_semaphore, nullptr);
            });
    }

    VkFenceCreateInfo uplaod_context_fence_info = vkinit::FenceCreateInfo();
    VK_CHECK(vkCreateFence(_device, &uplaod_context_fence_info, nullptr, &_upload_context.upload_fence));
    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Deleting upload fence in InitStructures()\n";
            vkDestroyFence(_device, _upload_context.upload_fence, nullptr); 
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

bool VulkanEngine::LoadShaderModule(const char* file_path, VkShaderModule* out_shader_module)
{
    // open file with cursor at end
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        return false;

    size_t file_size = (size_t)file.tellg();					// Gives size of file
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));	
    
    // Place cursor at begin of file and read in file
    file.seekg(0);
    file.read((char*)buffer.data(), file_size);
    file.close();

    // Create shader module 
    VkShaderModuleCreateInfo shader_module_info{};
    shader_module_info.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_info.pNext	= nullptr;
    shader_module_info.codeSize	= buffer.size() * sizeof(uint32_t);
    shader_module_info.pCode	= buffer.data();

    VkShaderModule shader_module;
    if (vkCreateShaderModule(_device, &shader_module_info, nullptr, &shader_module) != VK_SUCCESS)
        return false;

    *out_shader_module = shader_module;
    return true;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitPipeLines()
{    
    VkShaderModule color_mesh_shader;
    if (!LoadShaderModule("../../shaders/default_lit.frag.spv", &color_mesh_shader))
        std::cout << "Error when building the triangle fragment shader module\n";
    else
        std::cout << "Triangle fragment shader successfully loaded\n";

    VkShaderModule texture_mesh_shader;
    if (!LoadShaderModule("../../shaders/texture_lit.frag.spv", &texture_mesh_shader))
        std::cout << "Error when building the triangle fragment shader module\n";
    else
        std::cout << "Triangle fragment shader successfully loaded\n";

    VkShaderModule mesh_vertex_shader;
    if (!LoadShaderModule("../../shaders/tri_mesh.vert.spv", &mesh_vertex_shader))
        std::cout << "Error when building the triangle vertex shader module\n";
    else
        std::cout << "Triangle vertex shader successfully loaded\n";

    // build pipeline layout which controls i/o for shader(s)
    VkPushConstantRange push_constant{};
    push_constant.offset        = 0;
    push_constant.size          = sizeof(MeshPushConstants);
    push_constant.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT;

    // Pipeline layout for just triangles and monkey
    VkDescriptorSetLayout set_layouts[] = { _global_descriptor_set_layout, _object_descriptor_set_layout,  };
    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info    = vkinit::PipelineLayoutCreateInfo();
    mesh_pipeline_layout_info.pPushConstantRanges           = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount        = 1;
    mesh_pipeline_layout_info.setLayoutCount                = 2;
    mesh_pipeline_layout_info.pSetLayouts                   = set_layouts;
    VkPipelineLayout mesh_pipeline_layout{};
    VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &mesh_pipeline_layout));

    // Pipeline layout for triangles, monkey and lot_empire map
    VkDescriptorSetLayout texture_set_layout[] = {
        _global_descriptor_set_layout,
        _object_descriptor_set_layout,
        _single_texture_set_layout
    };
    VkPipelineLayoutCreateInfo texture_mesh_pipeline_layout_info = mesh_pipeline_layout_info;
    texture_mesh_pipeline_layout_info.setLayoutCount    = 3;
    texture_mesh_pipeline_layout_info.pSetLayouts       = texture_set_layout;
    VkPipelineLayout texture_mesh_pipeline_layout{};
    VK_CHECK(vkCreatePipelineLayout(_device, &texture_mesh_pipeline_layout_info, nullptr, &texture_mesh_pipeline_layout));

    VertexInputDescription vertex_description = Vertex::GetVertexDescription();

    PipelineBuilder pipeline_builder{};
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, mesh_vertex_shader));
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, color_mesh_shader));
    pipeline_builder._vertex_input_info         = vkinit::VertexInputStateCreateInfo();
    pipeline_builder._input_assembly_info       = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline_builder._viewport.x                = 0.0f;
    pipeline_builder._viewport.y                = 0.0f;
    pipeline_builder._viewport.width            = (float)_window_extent.width;
    pipeline_builder._viewport.height           = (float)_window_extent.height;
    pipeline_builder._viewport.minDepth         = 0.0f;
    pipeline_builder._viewport.maxDepth         = 1.0f;
    pipeline_builder._scissor.offset            = { 0, 0 };
    pipeline_builder._scissor.extent            = _window_extent;
    pipeline_builder._rasterizer_info           = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL); // Draw filled triangles
    pipeline_builder._multisampling_info        = vkinit::MultisamplingStateCreateInfo();
    pipeline_builder._color_blend_attachement   = vkinit::ColorBlendAttachmentState();
    pipeline_builder._pipeline_layout           = mesh_pipeline_layout;
    pipeline_builder._depth_stencil             = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    // Connect the pipeline builder to vertex input info
    pipeline_builder._vertex_input_info.pVertexAttributeDescriptions    = vertex_description.attributes.data();
    pipeline_builder._vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();
    pipeline_builder._vertex_input_info.pVertexBindingDescriptions      = vertex_description.bindings.data();
    pipeline_builder._vertex_input_info.vertexBindingDescriptionCount   = vertex_description.bindings.size();

    VkPipeline mesh_pipeline = pipeline_builder.BuildPipeline(_device, _render_pass);

    CreateMaterial(mesh_pipeline, mesh_pipeline_layout, "default_mesh");


    // Texture pipeline 
    pipeline_builder._shader_stages_infos.clear();
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, mesh_vertex_shader));
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, texture_mesh_shader));

    pipeline_builder._pipeline_layout   = texture_mesh_pipeline_layout;
    VkPipeline texture_pipeline         = pipeline_builder.BuildPipeline(_device, _render_pass);
    CreateMaterial(texture_pipeline, texture_mesh_pipeline_layout, "texture_mesh");
    

    // Destroy modules after used
    vkDestroyShaderModule(_device, mesh_vertex_shader, nullptr);
    vkDestroyShaderModule(_device, texture_mesh_shader, nullptr);
    vkDestroyShaderModule(_device, color_mesh_shader, nullptr);
     
    _main_deletion_queue.PushFunction([=]()
        {
            std::cout << "Deleting pipeline and pipeline layouts in InitPipelines()\n";
            vkDestroyPipeline(_device, mesh_pipeline, nullptr);
            vkDestroyPipeline(_device, texture_pipeline, nullptr);
            vkDestroyPipelineLayout(_device, mesh_pipeline_layout, nullptr);
            vkDestroyPipelineLayout(_device, texture_mesh_pipeline_layout, nullptr);
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::LoadMeshes()
{
    Mesh triangle_mesh{};
    triangle_mesh.vertices.resize(3);
    triangle_mesh.vertices[0].position = { 1.0f, 1.0f, 0.5f };
    triangle_mesh.vertices[1].position = { -1.0f, 1.0f, 0.5f };
    triangle_mesh.vertices[2].position = { 0.0f, -1.0f, 0.5f };

    triangle_mesh.vertices[0].color = { 0.0f, 1.0f, 0.0f };
    triangle_mesh.vertices[1].color = { 0.0f, 1.0f, 0.0f };
    triangle_mesh.vertices[2].color = { 0.0f, 1.0f, 0.0f };

    Mesh monkey_mesh{};
    monkey_mesh.LoadFromObj("../../assets/monkey_smooth.obj");

    Mesh lost_empire_mesh{};
    lost_empire_mesh.LoadFromObj("../../assets/lost_empire.obj");

    UploadMesh(monkey_mesh);
    UploadMesh(triangle_mesh);
    UploadMesh(lost_empire_mesh);

    _meshes["monkey"]       = monkey_mesh;
    _meshes["triangle"]     = triangle_mesh;
    _meshes["lost_empire"]  = lost_empire_mesh;

    _main_deletion_queue.PushFunction([=]()
        {
            std::cout << "Deleteing vertex buffers in LoadMeshes()\n";
            vmaDestroyBuffer(_allocator, monkey_mesh.vertex_buffer.buffer, monkey_mesh.vertex_buffer.allocation);
            vmaDestroyBuffer(_allocator, triangle_mesh.vertex_buffer.buffer, triangle_mesh.vertex_buffer.allocation);
            vmaDestroyBuffer(_allocator, lost_empire_mesh.vertex_buffer.buffer, lost_empire_mesh.vertex_buffer.allocation);
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::UploadMesh(Mesh& mesh)
{
    const size_t buffer_size = mesh.vertices.size() * sizeof(Vertex);
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size    = buffer_size;
    buffer_info.usage   = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo vmaalloc_info{};
    vmaalloc_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    AllocatedBuffer staging_buffer;

    VK_CHECK(vmaCreateBuffer(_allocator, &buffer_info, &vmaalloc_info, &staging_buffer.buffer, &staging_buffer.allocation, nullptr));

    // Map data to VkBuffer
    void* data;
    vmaMapMemory(_allocator, staging_buffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), buffer_size);
    vmaUnmapMemory(_allocator, staging_buffer.allocation);

    // Make GPU-side buffer
    VkBufferCreateInfo vertex_buffer_info{};
    vertex_buffer_info.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertex_buffer_info.pNext    = nullptr;
    vertex_buffer_info.size     = buffer_size;
    vertex_buffer_info.usage    = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    
    vmaalloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VK_CHECK(vmaCreateBuffer(_allocator, &vertex_buffer_info, &vmaalloc_info, &mesh.vertex_buffer.buffer, &mesh.vertex_buffer.allocation, nullptr));

    ImmediateSubmit([=](VkCommandBuffer command_buffer)
        {
            VkBufferCopy copy;
            copy.dstOffset = 0;
            copy.srcOffset = 0;
            copy.size = buffer_size;
            vkCmdCopyBuffer(command_buffer, staging_buffer.buffer, mesh.vertex_buffer.buffer, 1, &copy);
        });

    vmaDestroyBuffer(_allocator, staging_buffer.buffer, staging_buffer.allocation);
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

Material* VulkanEngine::CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
{
    Material material{};
    material.pipeline           = pipeline;
    material.pipeline_layout    = layout;
    _materials[name]            = material;
    return &_materials[name];
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

Material* VulkanEngine::GetMaterial(const std::string& name)
{
    auto it = _materials.find(name);
    if (it == _materials.end())
        return nullptr;
    else return &(*it).second;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

Mesh* VulkanEngine::GetMesh(const std::string& name)
{
    auto it = _meshes.find(name);
    if (it == _meshes.end())
        return nullptr;
    else return &(*it).second;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitScene()
{
    RenderObject monkey{};
    monkey.mesh             = GetMesh("monkey");
    monkey.material         = GetMaterial("default_mesh");
    monkey.transform_matrix = glm::mat4{ 1.0f };

    _renderables.push_back(monkey);

    for (int x = -20; x <= 20; x++)
    {
        for (int y = -20; y <= 20; y++)
        {
            glm::mat4 translation   = glm::translate(glm::mat4{ 1.0f }, glm::vec3(x, 0, y));
            glm::mat4 scale         = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.2, 0.2, 0.2));
            
            RenderObject triangle{};
            triangle.mesh               = GetMesh("triangle");
            triangle.material           = GetMaterial("default_mesh");
            triangle.transform_matrix   = translation * scale;

            _renderables.push_back(triangle);
        }
    }

    RenderObject lost_empire_map;
    lost_empire_map.mesh                = GetMesh("lost_empire");
    lost_empire_map.material            = GetMaterial("texture_mesh");
    lost_empire_map.transform_matrix    = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ 5, -10, 0 });

    _renderables.push_back(lost_empire_map);

    // Lost empire scene
    VkSamplerCreateInfo sampler_info = vkinit::SamplerCreateInfo(VK_FILTER_NEAREST);
    VkSampler blocky_sampler{};
    VK_CHECK(vkCreateSampler(_device, &sampler_info, nullptr, &blocky_sampler));
    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Destroy blocky sampler in InitScene()\n";
            vkDestroySampler(_device, blocky_sampler, nullptr); });

    Material* texture_matrix = GetMaterial("texture_mesh");
    VkDescriptorSetAllocateInfo set_allocation_info{};
    set_allocation_info.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    set_allocation_info.pNext               = nullptr;
    set_allocation_info.descriptorPool      = _descriptor_pool;
    set_allocation_info.descriptorSetCount  = 1;
    set_allocation_info.pSetLayouts         = &_single_texture_set_layout;
    vkAllocateDescriptorSets(_device, &set_allocation_info, &texture_matrix->texture_descriptor_set);

    VkDescriptorImageInfo image_info{};
    image_info.sampler      = blocky_sampler;
    image_info.imageView    = _loaded_textures["empire_diffuse"].image_view;
    image_info.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    
    VkWriteDescriptorSet write_texture_set = vkinit::WriteDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture_matrix->texture_descriptor_set, &image_info, 0);
    vkUpdateDescriptorSets(_device, 1, &write_texture_set, 0, nullptr);
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::DrawObjects(VkCommandBuffer command_buffer, RenderObject* first, int count)
{
    float framed = (_frame_number / 60.0f);
    
    _scene_parameters.ambient_color = { std::sin(framed), 0, std::cos(framed), 1 };
    
    // Copy data for descriptor set 0
    int frame_index = _frame_number % FRAME_OVERLAP;
    char* scene_data;
    vmaMapMemory(_allocator, _scene_parameters_buffer.allocation, (void**)&scene_data);
    scene_data += PadUniformBufferSize(sizeof(GPUSceneData)) * frame_index;
    memcpy(scene_data, &_scene_parameters, sizeof(GPUSceneData));
    vmaUnmapMemory(_allocator, _scene_parameters_buffer.allocation);

    // make model view matrix for renderable
    GPUCameraData gpu_camera{};
    gpu_camera.view_matrix          = _camera.view_matrix;
    gpu_camera.projection_matrix    = _camera.projection_matrix;
    gpu_camera.view_projection      = _camera.projection_matrix * _camera.view_matrix;

    void* data;
    vmaMapMemory(_allocator, GetCurrentFrame().camera_buffer.allocation, &data);
    memcpy(data, &gpu_camera, sizeof(GPUCameraData));
    vmaUnmapMemory(_allocator, GetCurrentFrame().camera_buffer.allocation);

    // Copy data for descriptor set 1
    void* object_data;
    vmaMapMemory(_allocator, GetCurrentFrame().object_buffer.allocation, &object_data);
    
    // cast object_data as GPUObjectData* and write into it manually
    GPUObjectData* object_shader_storage_buffer_object = (GPUObjectData*)object_data;
    for (int i = 0; i < count; i++)
    {
        RenderObject& object                                = first[i];
        object_shader_storage_buffer_object[i].model_matrix = object.transform_matrix;
    }
    
    vmaUnmapMemory(_allocator, GetCurrentFrame().object_buffer.allocation);

    Mesh*       last_mesh     = nullptr; // first[0].mesh;
    Material*   last_material = nullptr; // first[0].material;
    for (int i = 0; i < count; i++)
    {
        RenderObject& object = first[i];

        // only bind material if it doesn't match with the last one
        // TODO: sort render objects by pipeline and mesh to reduce the number of bind calls
        if (object.material != last_material)
        {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            last_material = object.material;

            uint32_t uniform_offset = PadUniformBufferSize(sizeof(GPUSceneData)) * frame_index;
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 0, 1, &GetCurrentFrame().global_descriptor_set, 1, &uniform_offset);

            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 1, 1, &GetCurrentFrame().object_descriptor_set, 0, nullptr);

            if (object.material->texture_descriptor_set != VK_NULL_HANDLE)
            {
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline_layout, 2, 1, &object.material->texture_descriptor_set, 0, nullptr);
            }
        }

        MeshPushConstants constants{};
        constants.render_matrix = object.transform_matrix;

        // upload mesh to GPU view push constants
        vkCmdPushConstants(command_buffer, object.material->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

        // only bind mesh if it is different than last mesh
        if (object.mesh != last_mesh)
        {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &object.mesh->vertex_buffer.buffer, &offset);
            last_mesh = object.mesh;
        }

        // Draw!
        vkCmdDraw(command_buffer, object.mesh->vertices.size(), 1, 0, i);
    }
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitCamera()
{
    _camera.yaw                 = 0.0;
    _camera.pitch               = 0.0;
    _camera.position            = glm::vec3{ 0.0, 1.0, 0.0 };
    _camera.front               = glm::vec3{ 0.0, 0.0, 1.0 };
    _camera.right               = glm::vec3{ 1.0, 0.0, 0.0 };
    _camera.up                  = glm::cross(_camera.right, _camera.up);
    _camera.view_matrix         = glm::lookAt(_camera.position, _camera.position + _camera.front, _camera.up);
    _camera.field_of_view       = 70.0f;
    _camera.projection_matrix   = glm::perspective(glm::radians(_camera.field_of_view), _aspect_ratio, 0.1f, 200.f);
    _camera.projection_matrix[1][1] *= -1.0;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

FrameData& VulkanEngine::GetCurrentFrame()
{
    return _frame_data[_frame_number % FRAME_OVERLAP];
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

AllocatedBuffer VulkanEngine::CreateBuffer(size_t allocation_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType   = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size    = allocation_size;
    buffer_info.usage   = usage;

    VmaAllocationCreateInfo vmaalloc_info{};
    vmaalloc_info.usage = memory_usage;

    AllocatedBuffer new_buffer{};

    VK_CHECK(vmaCreateBuffer(_allocator, &buffer_info, &vmaalloc_info, &new_buffer.buffer, &new_buffer.allocation, nullptr));

    return new_buffer;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitDescriptors()
{

    std::vector<VkDescriptorPoolSize> descriptor_pool_sizes =
    {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}
    };

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags         = 0;
    pool_info.maxSets       = 10;
    pool_info.poolSizeCount = (uint32_t)descriptor_pool_sizes.size();
    pool_info.pPoolSizes    = descriptor_pool_sizes.data();

    VK_CHECK(vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptor_pool));

    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Destroying descriptor pool from InitDescriptor()\n";
            vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr); 
        });

    // Descriptor sets layout binding for set 0
    VkDescriptorSetLayoutBinding camera_buffer_binding  = vkinit::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    VkDescriptorSetLayoutBinding scene_buffer_binding   = vkinit::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    VkDescriptorSetLayoutBinding bindings[]             = { camera_buffer_binding, scene_buffer_binding };

    // Descriptor sets layout binding for set 1
    VkDescriptorSetLayoutBinding object_bind = vkinit::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    
    // Descriptor set layouyt binding for set 2
    VkDescriptorSetLayoutBinding texture_set_binding = vkinit::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

    // Descriptor set layout info for set 0
    VkDescriptorSetLayoutCreateInfo set_info_0{};
    set_info_0.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info_0.pNext          = nullptr;
    set_info_0.bindingCount   = 2;
    set_info_0.flags          = 0;
    set_info_0.pBindings      = bindings;

    // Descriptor set layout info for set 1
    VkDescriptorSetLayoutCreateInfo set_info_1{};
    set_info_1.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info_1.pNext        = nullptr;
    set_info_1.bindingCount = 1;
    set_info_1.flags        = 0;
    set_info_1.pBindings    = &object_bind;

    // Descriptor set layout info for set 2
    VkDescriptorSetLayoutCreateInfo set_info_2{};
    set_info_2.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_info_2.pNext        = nullptr;
    set_info_2.bindingCount = 1;
    set_info_2.flags        = 0;
    set_info_2.pBindings    = &texture_set_binding;

    VK_CHECK(vkCreateDescriptorSetLayout(_device, &set_info_0, nullptr, &_global_descriptor_set_layout));
    VK_CHECK(vkCreateDescriptorSetLayout(_device, &set_info_1, nullptr, &_object_descriptor_set_layout));
    VK_CHECK(vkCreateDescriptorSetLayout(_device, &set_info_2, nullptr, &_single_texture_set_layout));

    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Deleting descriptor set layouts in InitDescriptors()\n";
            vkDestroyDescriptorSetLayout(_device, _global_descriptor_set_layout, nullptr);
            vkDestroyDescriptorSetLayout(_device, _object_descriptor_set_layout, nullptr);
            vkDestroyDescriptorSetLayout(_device, _single_texture_set_layout, nullptr); 
        });

    const size_t scene_parameter_buffer_size = FRAME_OVERLAP * PadUniformBufferSize(sizeof(GPUSceneData));
    _scene_parameters_buffer = CreateBuffer(scene_parameter_buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    _main_deletion_queue.PushFunction([=]()
        {
            vmaDestroyBuffer(_allocator, _scene_parameters_buffer.buffer, _scene_parameters_buffer.allocation);
        });

    VkDescriptorSetLayout set_layouts[] = { _global_descriptor_set_layout, _object_descriptor_set_layout };
    for (int i = 0; i < FRAME_OVERLAP; i++)
    {
        const int MAX_OBJECTS = 10000;
        _frame_data[i].camera_buffer = CreateBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        _frame_data[i].object_buffer = CreateBuffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        // allocate descriptor sets
        VkDescriptorSetAllocateInfo descriptor_set_allocation_info_0{};
        descriptor_set_allocation_info_0.sType                = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocation_info_0.pNext                = nullptr;
        descriptor_set_allocation_info_0.descriptorPool       = _descriptor_pool;
        descriptor_set_allocation_info_0.descriptorSetCount   = 2;
        descriptor_set_allocation_info_0.pSetLayouts          = &_global_descriptor_set_layout;

        VK_CHECK(vkAllocateDescriptorSets(_device, &descriptor_set_allocation_info_0, &_frame_data[i].global_descriptor_set));

        VkDescriptorSetAllocateInfo descriptor_set_allocation_info_1{};
        descriptor_set_allocation_info_1.sType               = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocation_info_1.pNext               = nullptr;
        descriptor_set_allocation_info_1.descriptorPool      = _descriptor_pool;
        descriptor_set_allocation_info_1.descriptorSetCount  = 1;
        descriptor_set_allocation_info_1.pSetLayouts         = &_object_descriptor_set_layout;

        VK_CHECK(vkAllocateDescriptorSets(_device, &descriptor_set_allocation_info_1, &_frame_data[i].object_descriptor_set));


        // Set up descriptor set 0
        VkDescriptorBufferInfo camera_buffer_info   = vkinit::DescriptorBufferInfo(_frame_data[i].camera_buffer.buffer, 0, sizeof(GPUCameraData));
        VkDescriptorBufferInfo scene_buffer_info    = vkinit::DescriptorBufferInfo(_scene_parameters_buffer.buffer, 0, sizeof(GPUSceneData));
        VkDescriptorBufferInfo object_buffer_info   = vkinit::DescriptorBufferInfo(_frame_data[i].object_buffer.buffer, 0, sizeof(GPUObjectData) * MAX_OBJECTS);

        VkWriteDescriptorSet write_camera   = vkinit::WriteDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _frame_data[i].global_descriptor_set, &camera_buffer_info, 0);
        VkWriteDescriptorSet write_scene    = vkinit::WriteDescriptorSet(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _frame_data[i].global_descriptor_set, &scene_buffer_info, 1);
        VkWriteDescriptorSet write_object   = vkinit::WriteDescriptorSet(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frame_data[i].object_descriptor_set, &object_buffer_info, 0);
        VkWriteDescriptorSet write_sets[]   = { write_camera, write_scene, write_object };
        
        vkUpdateDescriptorSets(_device, 3, write_sets, 0, nullptr);     // Upload descriptor set zeor

        _main_deletion_queue.PushFunction([=]()
            {
                std::cout << "Deleting object and camera buffers in InitDescriptors()\n";
                vmaDestroyBuffer(_allocator, _frame_data[i].camera_buffer.buffer, _frame_data[i].camera_buffer.allocation);
                vmaDestroyBuffer(_allocator, _frame_data[i].object_buffer.buffer, _frame_data[i].object_buffer.allocation);
            });
    }

}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

size_t VulkanEngine::PadUniformBufferSize(size_t original_size)
{
    size_t minimum_uniform_buffer_alignment = _gpu_properties.limits.minUniformBufferOffsetAlignment;
    size_t aligned_size                     = original_size;

    if (minimum_uniform_buffer_alignment > 0)
        aligned_size = (aligned_size + minimum_uniform_buffer_alignment - 1) & ~(minimum_uniform_buffer_alignment - 1);
    
    return aligned_size;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::ImmediateSubmit(std::function<void(VkCommandBuffer)>&& func)
{
    // Follows same steps as in VulkanEngine::Draw() functions
    VkCommandBufferAllocateInfo command_buffer_allocation_info = vkinit::CommandBufferAllocateInfo(_upload_context.command_pool, 1);
    VkCommandBuffer             command_buffer;
    VK_CHECK(vkAllocateCommandBuffers(_device, &command_buffer_allocation_info, &command_buffer));

    VkCommandBufferBeginInfo command_buffer_begin_info = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info));

    func(command_buffer);

    VK_CHECK(vkEndCommandBuffer(command_buffer));

    VkSubmitInfo submit_info = vkinit::SubmitInfo(&command_buffer);
    VK_CHECK(vkQueueSubmit(_graphics_queue, 1, &submit_info, _upload_context.upload_fence));

    vkWaitForFences(_device, 1, &_upload_context.upload_fence, true, 9999999999);
    vkResetFences(_device, 1, &_upload_context.upload_fence);
    vkResetCommandPool(_device, _upload_context.command_pool, 0);
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::LoadImages()
{
    Texture lost_empire_texture{};
    bool result = vkutil::LoadImageFromFile(*this, "../../assets/lost_empire-RGBA.png", lost_empire_texture.image);

    VkImageViewCreateInfo image_info = vkinit::ImageViewCreateInfo(VK_FORMAT_R8G8B8A8_SRGB, lost_empire_texture.image.image, VK_IMAGE_ASPECT_COLOR_BIT);
    vkCreateImageView(_device, &image_info, nullptr, &lost_empire_texture.image_view);
    _main_deletion_queue.PushFunction([=]() 
        { 
            std::cout << "Destroying lost empire image in LoadImage()\n";
            vkDestroyImageView(_device, lost_empire_texture.image_view, nullptr); 
        });

    _loaded_textures["empire_diffuse"] = lost_empire_texture;
}