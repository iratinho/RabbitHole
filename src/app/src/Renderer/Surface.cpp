#include "Renderer/Surface.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/RenderTarget.hpp"
#include <vulkan/vulkan_core.h>

Surface::Surface()
    : _swapChain(nullptr)
    , _renderContext(nullptr)
{}

void Surface::AllocateSurface(SurfaceCreateParams& params)
{
    // Copy the contents from the shared ptrs
    _surfaceRenderTarget.reset(params._swapChainRenderTarget.lock().get());
    _surfaceRenderTargetDepth.reset(params._swapChainRenderTargetDepth.lock().get());

    _swapChain = params._swapChain;
    _renderContext = params._renderContext;
}

void Surface::Present(const SurfacePresentParams& presentParams) const
{
    const auto swapChain = static_cast<VkSwapchainKHR>(_swapChain->GetNativeHandle());
    const auto waitSemaphore = static_cast<VkSemaphore>(presentParams._waitSemaphore);
    
    VkPresentInfoKHR present_info_khr;
    present_info_khr.pNext = nullptr;
    present_info_khr.pResults = nullptr;
    present_info_khr.pSwapchains = &swapChain;
    present_info_khr.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info_khr.swapchainCount = 1;
    present_info_khr.pImageIndices = &presentParams._frameIndex;
    present_info_khr.pWaitSemaphores = &waitSemaphore;
    present_info_khr.waitSemaphoreCount = 1;
    VkFunc::vkQueuePresentKHR(_renderContext->GetPresentQueueHandle(), &present_info_khr);
}

std::shared_ptr<RenderTarget> Surface::GetRenderTarget() {
    return _surfaceRenderTarget;
}

std::shared_ptr<RenderTarget> Surface::GetDepthRenderTarget() {
    return _surfaceRenderTargetDepth;
}
