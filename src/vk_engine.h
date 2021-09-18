// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vk_types.h>
#include <vk_initializers.h>

#include <vector>
#include <functional>
#include <unordered_map>
#include <string>

#include <vk_pipeline.h>
#include <deletion_queue.h>
#include <vk_mesh.h>
#include <vk_material.h>
#include <vk_renderObject.h>
#include <vk_frameData.h>

#include <vk_camera.h>
#include <vk_gpuSceneData.h>

constexpr unsigned int FRAME_OVERLAP = 2;  

struct UploadContext
{
    VkFence         upload_fence;
    VkCommandPool   command_pool;
};


struct Texture
{
    AllocatedImage  image;
    VkImageView     image_view;
};

class VulkanEngine {
public:
    // Core functionality
    void Init();	// initializes everything in the engine
    void Cleanup();	// shuts down the engine
    void Draw();	// draw loop
    void Run();		// run main loop

    // Shader Code
    bool LoadShaderModule(const char* file_path, VkShaderModule* out_shader_module);

    // Create Material and add to map
    Material* CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

    // Returns nullptr if can't be found
    Material*   GetMaterial(const std::string& name);
    Mesh*       GetMesh(const std::string& name);

    void LoadImages();

    // Returns FrameData struct
    FrameData& GetCurrentFrame();

    // Allocate buffer for each render frame
    AllocatedBuffer CreateBuffer(size_t allocation_size, VkBufferUsageFlags usage, VmaMemoryUsage memory_usage);

    // Pads scene data to fill alignment buffer requirement of GPU
    size_t PadUniformBufferSize(size_t original_size);

    void DrawObjects(VkCommandBuffer command_buffer, RenderObject* first, int count);

    void ImmediateSubmit(std::function<void(VkCommandBuffer)>&& func);

    // Vulkan API main interface
    VkInstance					_instance;			// Vulkan library Handle
    VkDebugUtilsMessengerEXT	_debug_messenger;	// Vulkan debug output
    VkPhysicalDevice			_chosen_gpu;		// GPU chosen as the default device <--- edit
    VkPhysicalDeviceProperties  _gpu_properties;
    VkDevice					_device;			// Vulkan device for commands
    VkSurfaceKHR				_surface;			// Vulkan window surface
    
    // Window object
    VkExtent2D			_window_extent{ 1700 , 900 };	// Window dimensions
    struct SDL_Window*	_window{ nullptr };
    float               _aspect_ratio = (float)_window_extent.width / (float)_window_extent.height;

    // Contains synchronization and command buffer
    FrameData _frame_data[FRAME_OVERLAP];

    // Default camera
    Camera          _camera;

    // Scene data 
    GPUSceneData    _scene_parameters;
    AllocatedBuffer _scene_parameters_buffer;

    // Swapchain functionality
    VkSwapchainKHR				_swapchain;					// Swap chain variable
    VkFormat					_swapchain_imagine_format;	// Image format expected from windowing system
    std::vector<VkImage>		_swapchain_images;			// Array for images in swapchain
    std::vector<VkImageView>	_swapchain_image_views;		// Array for image views in swapchain

    // Depth buffering 
    VkImageView     _depth_image_view;
    AllocatedImage  _depth_image;
    VkFormat        _depth_format;

    // GPU Queues
    VkQueue		_graphics_queue;		// Queue to submit rendering calls to
    uint32_t	_graphics_queue_family;	// Keeps track which family of queues we are submitting to

    // VkRenderPass 
    VkRenderPass				_render_pass;	// For organizing function calls and image data passed to graphics queue
    std::vector<VkFramebuffer>	_frame_buffers;	// Contains organized images

    // Setup Descriptor sets stuff
    VkDescriptorSetLayout   _global_descriptor_set_layout;
    VkDescriptorSetLayout   _object_descriptor_set_layout;
    VkDescriptorSetLayout   _single_texture_set_layout;
    VkDescriptorPool        _descriptor_pool;

    UploadContext _upload_context;

    // Memory allocation for vertex allocation
    VmaAllocator _allocator;

    // Queue for destruction calls
    DeletionQueue _main_deletion_queue;

    // Refactor to allow for rendering of arbitrary meshes
    std::vector<RenderObject>                   _renderables;
    std::unordered_map<std::string, Material>   _materials;
    std::unordered_map<std::string, Mesh>       _meshes;
    std::unordered_map<std::string, Texture>    _loaded_textures;

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
    void InitCamera();
    void InitDescriptors();
    void InitPipeLines();			// Initializes pipelines for objects we want to render
    void InitScene();
    void InitIMGUI();

    void LoadMeshes();
    void UploadMesh(Mesh& mesh);
};