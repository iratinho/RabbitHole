#pragma once
#include "Interfaces/RenderTargetInterface.h"
#include "Renderer/Texture.h"

class RenderTarget : public IRenderTargetInterface {
public:
    RenderTarget() = default;
    RenderTarget(RenderContext* render_context, const RenderTargetParams& params);
    ~RenderTarget() override;
    
    /** ----- ITextureInterface  ----- **/
    bool Initialize() override;
    void FreeResource() override;
    void SetTextureResource(void* resource) override;
    [[nodiscard]] unsigned GetWidth() const override;
    [[nodiscard]] unsigned GetHeight() const override;
    [[nodiscard]] void* GetView() const override;
    [[nodiscard]] const void* GetTextureResource() const override;
    [[nodiscard]] bool IsValidResource() const override;
    [[nodiscard]] std::shared_ptr<ITextureInterface> GetTexture() const override;

private:
    bool CreateView();
    RenderTargetParams _params;
    VkImageView _imageView{};
    std::shared_ptr<ITextureInterface> _texture;
    RenderContext* _renderContext;
};

