#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/TextureResource.hpp"


// Would it make sense to store 2 images in the render target? Like a color and depth/stencil target? where depth/stencil is optional
RenderTarget::RenderTarget(std::shared_ptr<RenderContext> renderContext, const RenderTargetParams& params)
    : _params(params)
    , _renderContext(renderContext) {
    _texture = std::make_shared<Texture2D>(_renderContext);
}

RenderTarget::~RenderTarget() {
    RenderTarget::FreeResource();
}

// TODO make static MakeRenderTarget for normal version and for a version that accepts a texture2D ?

bool RenderTarget::Initialize() {
    bool bExternalResource = _params._usageFlags == Rt_Swapchain; // I should improve this flags? Instead of swapchain flag i could use external
    bool bWasInitialized = _texture && _texture->Initialize(_params._textureParams._width, _params._textureParams._height, _params._textureParams.format, 1, _params._textureParams.flags, bExternalResource);
    return bWasInitialized;
}

void RenderTarget::FreeResource() {
    if(_renderContext && IsValidResource()) {
        _texture->FreeResource();
    }
}

void RenderTarget::SetTextureResource(void* resource) {
    if(_texture && _texture->GetResource()) {
        _texture->GetResource()->SetExternalResource(resource);
    }
}

unsigned RenderTarget::GetWidth() const {
    return _params._textureParams._width;
}

unsigned RenderTarget::GetHeight() const {
    return _params._textureParams._height;
}

const std::shared_ptr<TextureResource> RenderTarget::GetTextureResource() const {
    if(_texture) {
        return _texture->GetResource();
    }

    return nullptr;
}

bool RenderTarget::IsValidResource() const {
    // todo NEED TO FIX THIS CHECK
    return _texture != nullptr;
}

std::shared_ptr<Texture2D> RenderTarget::GetTexture() const {
    return _texture;
}
