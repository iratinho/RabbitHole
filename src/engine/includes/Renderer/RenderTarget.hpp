#pragma once
#include "Interfaces/RenderTargetInterface.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/Texture2D.hpp"

class Texture2D;
class TextureResource;

typedef enum RenderTargetUsageFlags
{
    Rt_None,
    Rt_Swapchain,
    Rt_UI
} RenderTargetUsageFlags;

typedef struct RenderTargetParams {
    TextureParameters _textureParams;
    RenderTargetUsageFlags _usageFlags;
} RenderTargetParams;

class RenderTarget {
public:
    RenderTarget() = default;
    RenderTarget(std::shared_ptr<RenderContext> renderContext, const RenderTargetParams& params);
    ~RenderTarget();
    
    /** ----- ITextureInterface  ----- **/
    bool Initialize();
    void CreateResource();
    void FreeResource();
    void SetTextureResource(void* resource);
    [[nodiscard]] unsigned GetWidth() const;
    [[nodiscard]] unsigned GetHeight() const;
    [[nodiscard]] const std::shared_ptr<TextureResource> GetTextureResource() const;
    [[nodiscard]] bool IsValidResource() const;
    [[nodiscard]] std::shared_ptr<Texture2D> GetTexture() const;
    [[nodiscard]] Format GetFormat() const {
        return _params._textureParams.pixelFormat;
    };

private:
    RenderTargetParams _params;
    std::shared_ptr<Texture2D> _texture;
    std::shared_ptr<RenderContext> _renderContext;
};

