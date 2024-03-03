#pragma once
#include "Interfaces/RenderTargetInterface.hpp"
#include "Renderer/render_context.hpp"

class Texture2D;
class TextureResource;

class RenderTarget : public IRenderTargetInterface {
public:
    RenderTarget() = default;
    RenderTarget(std::shared_ptr<RenderContext> renderContext, const RenderTargetParams& params);
    ~RenderTarget() override;
    
    /** ----- ITextureInterface  ----- **/
    bool Initialize() override;
    void FreeResource() override;
    void SetTextureResource(void* resource) override;
    [[nodiscard]] unsigned GetWidth() const override;
    [[nodiscard]] unsigned GetHeight() const override;
    [[nodiscard]] const std::shared_ptr<TextureResource> GetTextureResource() const override;
    [[nodiscard]] bool IsValidResource() const override;
    [[nodiscard]] std::shared_ptr<Texture2D> GetTexture() const override;
    [[nodiscard]] Format GetFormat() const {
        return _params._textureParams.format;
    };

private:
    RenderTargetParams _params;
    std::shared_ptr<Texture2D> _texture;
    std::shared_ptr<RenderContext> _renderContext;
};

