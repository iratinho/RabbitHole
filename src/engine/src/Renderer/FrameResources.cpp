#include "Renderer/FrameResources.hpp"
#include "Renderer/Fence.hpp"
#include "Renderer/CommandPool.hpp"
#include "Renderer/Surface.hpp"
#include "Renderer/render_context.hpp"

FrameResources::FrameResources(RenderContext* renderContext)
    : _renderContext(renderContext) {
}

bool FrameResources::Initialize() {
    _commandPool = std::make_shared<CommandPool>(_renderContext);
    _presentableSurface = std::make_shared<Surface>();
    _inFlightFence = std::make_shared<Fence>(_renderContext);

    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.flags = 0;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult result = VkFunc::vkCreateSemaphore(_renderContext->GetLogicalDeviceHandle(), &semaphoreCreateInfo, nullptr, &_renderFinishedSemaphore);
    
    // Since our pipeline is dynamic and it creates it self, we dont have enough information here to
    // asses how many frame buffers or samplers we will need per frame. Add a sensible number here for
    // each type of descriptor
    _descriptorPool = _renderContext->CreateDescriptorPool(1, 5);
    
    if(result != VK_SUCCESS || _descriptorPool == VK_NULL_HANDLE) {
        return false;
    }
    
    return true;
}
