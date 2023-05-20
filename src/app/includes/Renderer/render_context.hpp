#pragma once
#include "VulkanLoader.hpp"

#define VALIDATE_RETURN(op) if(!op) return false

class RenderSystem;
class Swapchain;

namespace app::window {
    class Window;
}

struct InitializationParams {
    bool validation_enabled_ = false; // Try to enable only in development
    uint32_t extension_count = 0;
    const char** instance_extensions = nullptr;
    app::window::Window* window_ = nullptr;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual bool Initialize(class RenderContext* const render_context, const InitializationParams& initialization_params) = 0;
    virtual VkCommandBuffer RecordCommandBuffers(uint32_t idx) = 0;
    virtual bool AllocateCommandBuffers(VkCommandPool command_pool, int pool_idx) = 0;
    virtual bool AllocateFrameBuffers(int command_idx, struct PresistentRenderTargets render_targets) = 0;
    virtual bool AllocateRenderingResources() = 0;
    virtual void HandleResize(int width, int height) = 0;
};

struct RenderImages {};

struct Position {
    float x;
    float y;
    float z;

    Position() = default;
    
    Position operator-(const Position& V) const
    {
        return Position(x - V.x, y - V.y, z - V.z);
    }

    Position operator+(const Position& V) const
    {
        return Position(x + V.x, y + V.y, z + V.z);
    }

    template<typename T>
    Position operator+(T Bias) const
    {
        return Position(x + (T)Bias, y + (T)Bias, z + (T)Bias);
    }
    
    // template <typename float>
    Position operator*(const float& Scale) const
    {
        return Position(x * static_cast<float>(Scale), y * static_cast<float>(Scale), z * static_cast<float>(Scale));
    }


    float SizeSquared() const
    {
        return x*x + y*y +z*z;
    }

    Position(float _x, float _y, float _z)
        : x(_x), y(_y), z(_z)
    {}
};

struct Color {
    float r;
    float g;
    float b;

    Color() = default;
    
    Color(float _x, float _y, float _z)
        : r(_x), g(_y), b(_z)
    {}

    
    Color operator+(const Color& V) const
    {
        return Color(r + V.r, g + V.g, b + V.b);
    }

    // template <typename float>
    Color operator*(const float& Scale) const
    {
        return Color(r * static_cast<float>(Scale), g * static_cast<float>(Scale), b * static_cast<float>(Scale));
    }

    
};
        
struct VertexData {
    Position position;
    Color color;
};

struct IndexRenderingData {
    size_t indices_offset;
    size_t vertex_data_offset;
    uint32_t indices_count; 
    VkBuffer buffer;
};

struct ImageCreateInfo {
    uint32_t width;
    uint32_t height;
    uint32_t mip_count;
    VkFormat format;
    VkImageUsageFlags usage_flags;
    bool is_depth = false;
};

struct ImageResources {
    VkImageView image_view;
    VkImage image;
};

struct SwapchainImage {
    VkImage* image;
    // RenderTarget* color_render_target;
    // RenderTarget* depth_render_target;
};

struct PhysicalDeviceInfo {
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties device_properties;
    uint64_t score;
    uint32_t queue_family_index; // The queue family index that has Graphics and Compute operations
    uint32_t graphics_queue_family_index;
    uint32_t compute_queue_family_index;
    uint32_t presentation_queue_family_index;
    std::unordered_set<const char*> extensions;
    VkPhysicalDeviceFeatures features;
    VkQueue graphics_queue;
    VkQueue present_queue;
};
    
class RenderContext {
public:
    RenderContext() = default;
    RenderContext(RenderSystem* renderSystem) : m_renderSystem(renderSystem) {}

    bool Initialize(const InitializationParams& initialization_params);

    RenderSystem* GetRenderSystem() const { return m_renderSystem; }

    bool CreateShader(const char* shader_path, VkShaderStageFlagBits shader_stage, VkPipelineShaderStageCreateInfo& shader_stage_create_info) const;

    bool AcquireNextImage(VkSwapchainKHR swapchain, uint32_t& swapchainImageIndex, VkSemaphore swapchainSemaphore);
    
    app::window::Window* GetWindow() const { return initialization_params_.window_; }

    VkDevice GetLogicalDeviceHandle() const { return logical_device_; }

    VkPhysicalDevice GetPhysicalDeviceHandle() { return device_info_.physical_device; }

    uint32_t GetGraphicsQueueIndex() { return device_info_.graphics_queue_family_index; }

    VkQueue GetGraphicsQueueHandle() { return device_info_.graphics_queue; }

    VkQueue GetPresentQueueHandle() { return device_info_.present_queue; }

    VkCommandPool GetPersistentCommandPool() const { return persistent_command_pool_; }

    Swapchain* GetSwapchain() const { return m_swapchain; }

    VkExtent2D GetSwapchainExtent() const;
    
    void MarkSwapchainDirty();
    
    bool CreateIndexedRenderingBuffer(std::vector<uint32_t> indices, std::vector<VertexData> vertex_data, VkCommandPool command_pool, IndexRenderingData& index_rendering_data);

    void DestroyImageView(VkImageView image_view);

    void DestroyImage(VkImage image);

    void DestroyFrameBuffer(VkFramebuffer framebuffer);

    void DestroyCommandPool(VkCommandPool command_pool);

    VkFence AllocateFence(VkFenceCreateInfo fence_create_info);
    
    void ResetFence(VkFence fence);

    void WaitFence(VkFence fence);

    VkCommandPool CreateCommandPool();

    void ResetCommandPool(VkCommandPool commandPool);
    
    VkCommandBuffer CreateCommandBuffer(void* commandPool);

    bool BeginCommandBuffer(void* commandBuffer);

    bool EndCommandBuffer(void* commandBuffer);

    bool AllocateBuffer(size_t allocationSize, VkBuffer& buffer, VkDeviceMemory& bufferMemory, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags);

    void* LockBuffer(VkDeviceMemory bufferMemory, size_t allocationSize) const;
    
    void UnlockBuffer(VkDeviceMemory bufferMemory) const;

    void CopyBuffer(VkCommandBuffer commandBuffer, VkBuffer src, VkBuffer dest, size_t allocationSize);
    
    /**
    * The buffer memory requirements has a field called "memoryTypeBits" that tell us the required memory type
    * for this specific buffer. The ideia is to iterate over the memory types returned by the vkGetPhysicalDeviceMemoryProperties
    * and find the memory type index that matches our requirement.
    *
    *  For memory properties we are using VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT but this is temporary
    * since this is a memory region that is visible for cpu/gpu but we would prefer to use VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT since it
    * will be more performant and this data does not change all the time
    * 
    * The documentation has a nice example on how to do that:
    * https://registry.khronos.org/vulkan/specs/1.3-khr-extensions/html/vkspec.html#memory-device-bitmask-list
    *
    */
    int FindMemoryTypeIndex(int memory_property_flag_bits, VkMemoryRequirements memory_requirements);

    bool CreateSwapChain(VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages);
    
private:
    bool CreateVulkanInstance();
    bool PickSuitableDevice();
    bool CreateLogicalDevice();
    bool CreateWindowSurface();
    // This pool is used for transfer operations
    bool CreatePersistentCommandPool();

    InitializationParams initialization_params_;
    VkInstance instance_;
    uint32_t loader_version_;
    PhysicalDeviceInfo device_info_;
    VkDevice logical_device_;
    VkSurfaceKHR surface_;
    VkCommandPool persistent_command_pool_;
    VulkanLoader vulkan_loader_;

    Swapchain* m_swapchain;
    RenderSystem* m_renderSystem;
};
