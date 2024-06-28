#pragma once
#include "Renderer/Event.hpp"
#include "vulkan/vulkan_core.h"

class VKEvent : public Event {
public:
    
    VkSemaphore GetVkSemaphore() {
        return _semaphore;
    };
    
protected:
    void Initialize() override;
    
private:
    VkSemaphore _semaphore = VK_NULL_HANDLE;
};
