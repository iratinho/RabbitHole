#pragma once
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"

class Device;

class VKImageBuffer : public VKBuffer {
public:
    VKImageBuffer(Device* device, std::weak_ptr<TextureResource> resource);
    
    virtual void Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) override;

protected:
    std::weak_ptr<TextureResource> _resource;
    
private:
    VkDeviceMemory MakeImageBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryFlags, VkImage image);
};
