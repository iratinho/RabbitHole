#pragma once
#include "Renderer/TextureView.hpp"
#include "vulkan/vulkan_core.h"

class RenderContext;

class VkTextureView : public TextureView {
public:
    using TextureView::TextureView;
    virtual void CreateView(Format format, const Range& levels, TextureType textureType);
    virtual void FreeView();
    
    [[nodiscard]] VkImageView GetImageView() const {
        return _imageView;
    }
    
private:
    VkImageView _imageView = VK_NULL_HANDLE;
};
