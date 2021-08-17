// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

#include <vector>
#include <functional>

#include <vk_pipeline.h>
#include <deletion_queue.h>

class VulkanEngine {
public:
    // Core functionality
    void Init();	// initializes everything in the engine
    void Cleanup();	// shuts down the engine
    void Draw();	// draw loop
    void Run();		// run main loop

    // Shader Code
    bool LoadShaderModule(const char* file_path, VkShaderModule* out_shader_module);

    // Vulkan API main interface
    VkInstance					_instance;			// Vulkan library Handle
    VkDebugUtilsMessengerEXT	_debug_messenger;	// Vulkan debug output
    VkPhysicalDevice			_chosenGPU;			// GPU chosen as the default device <--- edit
    VkDevice					_device;			// Vulkan device for commands
    VkSurfaceKHR				_surface;			// Vulkan window surface
    
    // Window object
    VkExtent2D			_window_extent{ 1700 , 900 };	// Window dimensions
    struct SDL_Window*	_window{ nullptr };

    // Swapchain functionality
    VkSwapchainKHR				_swapchain;					// Swap chain variable
    VkFormat					_swapchain_imagine_format;	// Image format expected from windowing system
    std::vector<VkImage>		_swapchain_images;			// Array for images in swapchain
    std::vector<VkImageView>	_swapchain_image_views;		// Array for image views in swapchain

    // Command Pool and Buffer
    VkCommandPool	_command_pool;		
    VkCommandBuffer	_command_buffer;	// Stores functions calls passed to GPU

    // GPU Queues
    VkQueue		_graphics_queue;		// Queue to submit rendering calls to
    uint32_t	_graphics_queue_family;	// Keeps track which family of queues we are submitting to

    // VkRenderPass 
    VkRenderPass				_render_pass;	// For organizing function calls and image data passed to graphics queue
    std::vector<VkFramebuffer>	_frame_buffers;	// Contains organized images

    // GPU Synchronoization 
    VkSemaphore _present_semaphore;
    VkSemaphore _render_semaphore;
    VkFence		_render_fence;

    // Grahics pipeline necessities
    VkPipelineLayout    _triangle_pipeline_layout;
    VkPipeline          _triangle_pipeline;
    VkPipeline          _colored_triangle_pipeline;

    // Memory allocation for vertex allocation
    VmaAllocator _allocator;

    // Queue for destruction calls
    DeletionQueue _main_deletion_queue;

    // Bool for successful initialization
    bool _isInitialized{ false };

    int _frame_number {0};

    // For toggling between shaders
    int _selected_shader;

private:
    void InitVulkan();
    void InitSwapchain();
    void InitCommands();
    void InitDefaultRenderPass();
    void InitFrameBuffers();
    void InitSyncStructure();		// Initializes semaphores and fences for GPU synchronization
    void InitPipeLines();			// Initializes pipelines for objects we want to render
};