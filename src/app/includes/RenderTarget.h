#pragma once
#include "Texture.h"

// At the moment a render target is a texture, in the future create a parent class as Texture so that we can have a separation RenderTarget <- Texture
class RenderTarget : public Texture {
public:
    RenderTarget() = default;
    RenderTarget(RenderContext* render_context, const TextureParams& params);
    RenderTarget(Texture&& texture);

    ~RenderTarget();

    bool Initialize();
    void FreeResource(bool only_view = false);
    uint32_t GetHeight() { return params_.height; }
    uint32_t GetWidth() { return params_.width; }
    
    _forceinline VkImageView GetRenderTargetView() const { return image_view_; };

private:
    bool CreateResource();
    VkImageView image_view_;
};

