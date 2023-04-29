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
    [[nodiscard]] inline unsigned GetWidth() const override;
    [[nodiscard]] inline unsigned GetHeight() const override;
    [[nodiscard]] inline void* GetView() const override;
    [[nodiscard]] inline const void* GetTextureResource() const override;
    [[nodiscard]] inline bool IsValidResource() const override;
    [[nodiscard]] inline std::shared_ptr<ITextureInterface> GetTexture() const override;

private:
    bool CreateView();
    RenderTargetParams _params;
    VkImageView _imageView{};
    std::shared_ptr<ITextureInterface> _texture;
    RenderContext* _renderContext;
};

