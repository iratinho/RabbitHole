#pragma once
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/TextureResource.hpp"

class RenderContext;

class VkTextureResource : public TextureResource {
public:
    using TextureResource::TextureResource;
    ~VkTextureResource() override {
        FreeResource();
    };
    
    void CreateResource() override;
    
    void SetExternalResource(void* handle) override;

    void FreeResource() override;
    
    bool HasValidResource() override;
    
    void* Lock() override;
    
    void Unlock() override;

//Vulkan Specific
public:
    VkImage GetImage();
    
private:
    VkImage _image = VK_NULL_HANDLE;
    VkDeviceMemory _memory = VK_NULL_HANDLE;
};


