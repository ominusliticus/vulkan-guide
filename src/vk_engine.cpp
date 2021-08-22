
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
    _window = SDL_CreateWindow(
        "Vulkan Window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        _window_extent.width,
        _window_extent.height,
        window_flags);


    InitVulkan();				// load Vulkan core structures
    InitSwapchain();			// create swapchain
    InitCommands();				// create command pool(s) and command buffers
    InitDefaultRenderPass();	// Self evident name...no more commenting from here
    InitFrameBuffers();
    InitSyncStructure();		// Synchronozation for GPU: fences and semaphores
    InitPipeLines();
    LoadMeshes();
    InitScene();
    InitCamera();

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
    VK_CHECK(vkWaitForFences(_device, 1, &_render_fence, true, one_second));
    VK_CHECK(vkResetFences(_device, 1, &_render_fence));

    // Request image from swap chain
    uint32_t swapchain_image_index;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, one_second, _present_semaphore, nullptr, &swapchain_image_index));

    // Reset command buffer for new render pass
    VK_CHECK(vkResetCommandBuffer(_command_buffer, 0));

    // Creating Begin Command Buffer instance
    VkCommandBufferBeginInfo command_buffer_begin_info{};
    command_buffer_begin_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext				= nullptr;
    command_buffer_begin_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    command_buffer_begin_info.pInheritanceInfo	= nullptr;

    VK_CHECK(vkBeginCommandBuffer(_command_buffer, &command_buffer_begin_info));

    // Render pass that flashes a clear-color over time
    VkClearValue clear_value;
    float flash = std::abs(std::sin(_frame_number / 12.0f));
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
    vkCmdBeginRenderPass(_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    DrawObjects(_command_buffer, _renderables.data(), _renderables.size());

    // End rendering pass
    vkCmdEndRenderPass(_command_buffer);
    VK_CHECK(vkEndCommandBuffer(_command_buffer));

    // Prepare to send to queue. Wait for _present_semaphore to finish before starting _render_semaphore
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
    submit_info.pWaitDstStageMask	= &wait_stage;
    submit_info.waitSemaphoreCount	= 1;
    submit_info.pWaitSemaphores		= &_present_semaphore;
    
    submit_info.signalSemaphoreCount	= 1;
    submit_info.pSignalSemaphores		= &_render_semaphore;

    submit_info.commandBufferCount	= 1;
    submit_info.pCommandBuffers		= &_command_buffer;

    // Submit command command buffers to queue and execute
    // _render_fence will now block until the graphics command is done
    VK_CHECK(vkQueueSubmit(_graphics_queue, 1, &submit_info, _render_fence));

    // Display image to screen: wait until _render_semaphore is done 
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;

    present_info.swapchainCount		= 1;
    present_info.pSwapchains		= &_swapchain;
    present_info.waitSemaphoreCount	= 1;
    present_info.pWaitSemaphores	= &_render_semaphore;
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


                int x, y;
                uint32_t button = SDL_GetMouseState(&x, &y);
                std::cout << "Mouse is at: ( " << x << " , " << y << " )\n";
                // WASD implementation
                float speed = 0.1f;
                if (e.key.keysym.sym == SDLK_LSHIFT || e.key.keysym.sym == SDLK_RSHIFT)
                {
                    SDL_PumpEvents();
                    if (e.key.keysym.sym == SDLK_a)
                    {
                        _camera.position.x += speed;
                    }
                    if (e.key.keysym.sym == SDLK_s)
                    {
                        _camera.position.z += -speed;
                    }
                    if (e.key.keysym.sym == SDLK_d)
                    {
                        _camera.position.x += -speed;
                    }
                    if (e.key.keysym.sym == SDLK_w)
                    {
                        _camera.position.z += speed;
                    }
                }
                else
                {
                    speed *= 0.5;
                    if (e.key.keysym.sym == SDLK_a)
                    {
                        _camera.position.x += speed;
                    }
                    if (e.key.keysym.sym == SDLK_s)
                    {
                        _camera.position.z += -speed;
                    }
                    if (e.key.keysym.sym == SDLK_d)
                    {
                        _camera.position.x += -speed;
                    }
                    if (e.key.keysym.sym == SDLK_w)
                    {
                        _camera.position.z += speed;
                    }
                }
            }
        }

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
    _chosenGPU = physical_device.physical_device;

    // use vkb to get graphics queue(s)
    _graphics_queue			= vkb_device.get_queue(vkb::QueueType::graphics).value();
    _graphics_queue_family	= vkb_device.get_queue_index(vkb::QueueType::graphics).value();

    // Initialize allocation buffers
    VmaAllocatorCreateInfo allocator_info{};
    allocator_info.physicalDevice   = _chosenGPU;
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
            vkDestroyImageView(_device, _depth_image_view, nullptr);
            vmaDestroyImage(_allocator, _depth_image.image, _depth_image.allocation);
        });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitSwapchain()
{
    vkb::SwapchainBuilder swapchain_builder{ _chosenGPU, _device, _surface };
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

    _main_deletion_queue.PushFunction([=]() { vkDestroySwapchainKHR(_device, _swapchain, nullptr); });
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitCommands()
{
    // Create command pool and get graphics queue
    VkCommandPoolCreateInfo command_pool_info = vkinit::CommandPoolCreateInfo(_graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    VK_CHECK(vkCreateCommandPool(_device, &command_pool_info, nullptr, &_command_pool));
    
    // Create command buffer
    VkCommandBufferAllocateInfo command_buffer_info = vkinit::CommandBufferAllocateInfo(_command_pool, 1);
    VK_CHECK(vkAllocateCommandBuffers(_device, &command_buffer_info, &_command_buffer));

    _main_deletion_queue.PushFunction([=]() {vkDestroyCommandPool(_device, _command_pool, nullptr); });
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
    frame_buffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_info.pNext = nullptr;
    frame_buffer_info.renderPass = _render_pass;
    frame_buffer_info.attachmentCount = 1;
    frame_buffer_info.width = _window_extent.width;
    frame_buffer_info.height = _window_extent.height;
    frame_buffer_info.layers = 1;

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
                vkDestroyFramebuffer(_device, _frame_buffers[i], nullptr); 
                vkDestroyImageView(_device, _swapchain_image_views[i], nullptr);
            });
    }

}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::InitSyncStructure()
{
    // Create fence
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = nullptr;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(_device, &fence_info, nullptr, &_render_fence));

    _main_deletion_queue.PushFunction([=]() {vkDestroyFence(_device, _render_fence, nullptr); });

    // Create semaphores
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = nullptr;
    semaphore_info.flags = 0;

    VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_present_semaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphore_info, nullptr, &_render_semaphore));

    _main_deletion_queue.PushFunction([=]
        {
            vkDestroySemaphore(_device, _present_semaphore, nullptr);
            vkDestroySemaphore(_device, _render_semaphore, nullptr);
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

    VkShaderModule colored_triangle_fragment_shader;
    if (!LoadShaderModule("../../shaders/colored_triangle.frag.spv", &colored_triangle_fragment_shader))
        std::cout << "Error when building the triangle fragment shader module\n";
    else
        std::cout << "Triangle fragment shader successfully loaded\n";

    VkShaderModule mesh_vertex_shader;
    if (!LoadShaderModule("../../shaders/tri_mesh.vert.spv", &mesh_vertex_shader))
        std::cout << "Error when building the triangle vertex shader module\n";
    else
        std::cout << "Triangle vertex shader successfully loaded\n";

    // build pipeline layout which controls i/o for shader(s)
    VkPipelineLayout mesh_pipeline_layout{};
    VkPipelineLayoutCreateInfo mesh_pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
    VkPushConstantRange push_constant{};
    push_constant.offset        = 0;
    push_constant.size          = sizeof(MeshPushConstants);
    push_constant.stageFlags    = VK_SHADER_STAGE_VERTEX_BIT;
    mesh_pipeline_layout_info.pPushConstantRanges       = &push_constant;
    mesh_pipeline_layout_info.pushConstantRangeCount    = 1;
    VK_CHECK(vkCreatePipelineLayout(_device, &mesh_pipeline_layout_info, nullptr, &mesh_pipeline_layout));

    VertexInputDescription vertex_description = Vertex::GetVertexDescription();

    PipelineBuilder pipeline_builder{};
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, mesh_vertex_shader));
    pipeline_builder._shader_stages_infos.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colored_triangle_fragment_shader));
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

    // Destroy modules after used
    vkDestroyShaderModule(_device, mesh_vertex_shader, nullptr);
    vkDestroyShaderModule(_device, colored_triangle_fragment_shader, nullptr);
     
    _main_deletion_queue.PushFunction([=]()
        {
            vkDestroyPipeline(_device, mesh_pipeline, nullptr);
            vkDestroyPipelineLayout(_device, mesh_pipeline_layout, nullptr);
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

    UploadMesh(monkey_mesh);
    UploadMesh(triangle_mesh);

    _meshes["monkey"]   = monkey_mesh;
    _meshes["triangle"] = triangle_mesh;
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::UploadMesh(Mesh& mesh)
{
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = mesh.vertices.size() * sizeof(Vertex);
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaalloc_info{};
    vmaalloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(_allocator, &buffer_info, &vmaalloc_info, &mesh.vertex_buffer.buffer, &mesh.vertex_buffer.allocation, nullptr));

    _main_deletion_queue.PushFunction([=]() {vmaDestroyBuffer(_allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation); });

    // Map data to VkBuffer
    void* data;
    vmaMapMemory(_allocator, mesh.vertex_buffer.allocation, &data);
    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(_allocator, mesh.vertex_buffer.allocation);
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
            
            RenderObject triangle;
            triangle.mesh               = GetMesh("triangle");
            triangle.material           = GetMaterial("default_mesh");
            triangle.transform_matrix   = translation * scale;

            _renderables.push_back(triangle);
        }
    }
}

// .....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....oooOO0OOooo.....

void VulkanEngine::DrawObjects(VkCommandBuffer command_buffer, RenderObject* first, int count)
{
    // make model view matrix for renderable

    Mesh*       last_mesh     = nullptr; // first[0].mesh;
    Material*   last_material = nullptr; // first[0].material;
    for (int i = 0; i < count; i++)
    {
        RenderObject& object = first[i];

        // only bind mesh if it doesn't match with the last one
        if (object.material != last_material)
        {
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            last_material = object.material;
        }

        glm::mat4 model_matrix  = object.transform_matrix;
        glm::mat4 mesh_matrix   = _camera.GetCamera(model_matrix);

        MeshPushConstants constants{};
        constants.render_matrix = mesh_matrix;

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
        vkCmdDraw(command_buffer, object.mesh->vertices.size(), 1, 0, 0);
    }
}

void VulkanEngine::InitCamera()
{
    _camera.position            = glm::vec3{ 0.0, -1.0, 0.0 };
    _camera.field_of_view       = 70.0f;
    _camera.projection_matrix   = glm::perspective(glm::radians(_camera.field_of_view), _aspect_ratio, 0.1f, 200.f);
    _camera.projection_matrix[1][1] *= -1.0;
}