#pragma once
#include "vulkan/vulkan.hpp"
#include "Core/Utils.hpp"
#include "Renderer/Swapchain.hpp"

class VKSwapchain : public Swapchain {
public:
    explicit VKSwapchain(Device *device)
        : Swapchain(device)
    {}

    bool Initialize() override;
    void Shutdown() override;

    bool PrepareNextImage() override;
    std::uint8_t GetImageCount() override { return _imageCount; }
    std::shared_ptr<Texture2D> GetTexture(ESwapchainTextureType_ type) override;
    std::shared_ptr<Event> GetSyncEvent() override;

public:
    [[nodiscard]] VkSwapchainKHR GetVkSwapchainKHR() const { return _swapchain; }
    [[nodiscard]] std::uint8_t GetCurrentImageIdx() const { return _currentIdx; }

private:
    bool CreateRenderTargets();
    bool CreateSyncPrimitives();

private:
    std::uint32_t _currentIdx = 0;
    std::uint32_t _imageCount = 2;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> _images;

    CircularBuffer<std::shared_ptr<Event>, 2> _events;

    std::vector<std::shared_ptr<Texture2D>> _colorTextures;
    std::vector<std::shared_ptr<Texture2D>> _depthTextures;
};
