#pragma once
#include "Renderer/VulkanLoader.h"

class RenderContext;

struct TextureParams {
    uint32_t width = 0;
    uint32_t height = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t sample_count = 0;
    bool has_swapchain_usage:1 = false;
};

class Texture {
public:
    Texture() = default;
    Texture(RenderContext* render_context, TextureParams params);
    Texture(RenderContext* render_context, TextureParams params, VkImage image);
    Texture(Texture&& texture);
        
    bool Initialize();
    void FreeResource();

    VkImage GetResource() const { return image_; };
    bool IsValidResource() const { return image_ != VK_NULL_HANDLE; };

    uint32_t GetWidth() const { return params_.width; }
    uint32_t GetHeight() const { return params_.height; }
        
protected:
    TextureParams params_;
    RenderContext* render_context_;

private:
    VkImage image_;
};
