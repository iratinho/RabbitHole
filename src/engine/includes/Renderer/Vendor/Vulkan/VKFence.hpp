#pragma once
#include "Renderer/Fence.hpp"
#include "vulkan/vulkan_core.h"

class VKFence : public Fence {
public:
    VkFence GetVkFence() {
        return _fence;
    };
    
public:
    void Wait() override;
    
protected:
    void Initialize() override;
    
private:
    VkFence _fence = VK_NULL_HANDLE;
};
