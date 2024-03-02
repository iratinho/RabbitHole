#pragma once
#include "Renderer/Buffer.hpp"
#include "vulkan/vulkan.hpp"

class VKBuffer : public Buffer {
public:
    void Initialize() override;
    
    void* LockBuffer() override;
    
    void UnlockBuffer() override;
    
    VkBuffer GetHostBuffer() { return _stagingBuffer; }
    VkBuffer GetLocalBuffer() { return _localBuffer; }
    
    VkDeviceMemory GetHostMemory() { return _stagingBufferMemory; }
    VkDeviceMemory GetLocalMemory() { return _localBufferMemory; }
    
private:
    void* _cpuData = nullptr;
    
    VkBuffer _stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _stagingBufferMemory = VK_NULL_HANDLE;
    
    VkBuffer _localBuffer = VK_NULL_HANDLE;
    VkDeviceMemory _localBufferMemory = VK_NULL_HANDLE;

};
