#pragma once
#include "Renderer/Device.hpp"
#include "Renderer/VulkanLoader.hpp"

class Swapchain;

class VKDevice : public Device {
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
    
public:
    bool Initialize() override;
    void Shutdown() override;

    VkDevice GetLogicalDeviceHandle() const {
        return logical_device_;
    }
    
    uint32_t GetGraphicsQueueIndex() {
        return device_info_.graphics_queue_family_index;
    }
    
    VkQueue GetPresentQueueHandle() { return device_info_.present_queue; }
    
    VkQueue GetGraphicsQueueHandle() { return device_info_.graphics_queue; }
    
    VkPhysicalDevice GetPhysicalDeviceHandle() { return device_info_.physical_device; }
    
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

    VkDescriptorPool CreateDescriptorPool(unsigned int uniformsCount, unsigned int samplersCount);
    
    // TODO: Need to work on the swapchaiin abstraction, this should be moved to there
    bool CreateSwapChain(VkSwapchainKHR& swapchain, std::vector<VkImage>& swapchainImages);
    bool AcquireNextImage(VkSwapchainKHR swapchain, uint32_t& swapchainImageIndex, VkSemaphore swapchainSemaphore);
    
private:
    bool CreateVulkanInstance();
    bool PickSuitableDevice();
    bool CreateLogicalDevice();
    bool CreateWindowSurface();
    bool CreatePersistentCommandPool();
    
private:
    VkInstance instance_;
    VkDevice logical_device_;
    VkSurfaceKHR surface_;
    VkCommandPool persistent_command_pool_;
    std::shared_ptr<Swapchain> m_swapchain;
    PhysicalDeviceInfo device_info_;
    
    uint32_t _winExtensionCount = 0;
    bool _validationEnabled = false; // Try to enable only in development
    const char** _instanceExtensions = nullptr;

    // Can this be inside cpp?
    VulkanLoader vulkan_loader_;
    uint32_t loader_version_;
};
