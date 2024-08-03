#pragma once
#include "Renderer/Buffer.hpp"
#include "vulkan/vulkan.hpp"

class Texture2D;

class VKBuffer : public Buffer {
public:
    virtual void Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) override;
    
    void* LockBuffer() override;
    
    void UnlockBuffer() override;
    
    VkBuffer GetHostBuffer() { return _stagingBuffer; }
    VkBuffer GetLocalBuffer() { return _localBuffer; }
    
    VkDeviceMemory GetHostMemory() { return _stagingBufferMemory; }
    VkDeviceMemory GetLocalMemory() { return _localBufferMemory; }
    
protected:
    std::pair<VkBuffer, VkDeviceMemory> MakeBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryFlags);

    void* _cpuData = nullptr;
    
    VkBuffer _stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _stagingBufferMemory = VK_NULL_HANDLE;
    
    VkBuffer _localBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _localBufferMemory = VK_NULL_HANDLE;
};
