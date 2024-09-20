#pragma once
#include "Renderer/TextureView.hpp"
#include "vulkan/vulkan_core.h"

class RenderContext;

class VKTextureView : public TextureView {
public:
    using TextureView::TextureView;

    ~VKTextureView() override {
        VKTextureView::FreeView();
    }

    void CreateView(Format format, const Range& levels, TextureType textureType) override;

    void FreeView() override;
    
    [[nodiscard]] VkImageView GetImageView() const {
        return _imageView;
    }
    
private:
    VkImageView _imageView = VK_NULL_HANDLE;
};
