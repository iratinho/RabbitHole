#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/TextureResource.hpp"

// TODO: I need to improve this class.

// Would it make sense to store 2 images in the render target? Like a color and depth/stencil target? where depth/stencil is optional
RenderTarget::RenderTarget(std::shared_ptr<RenderContext> renderContext, const RenderTargetParams& params)
    : _params(params)
    , _renderContext(renderContext) {
    if(_params._usageFlags == Rt_Swapchain) {
        _texture = Texture2D::MakeFromExternalResource(_params._textureParams._width, _params._textureParams._height, _params._textureParams.pixelFormat, 1, _params._textureParams.flags); // This wont make sense for other types of render targets were the resource is managed by us
    } else {
        _texture = Texture2D::MakeTexturePass(_params._textureParams._width, _params._textureParams._height, _params._textureParams.pixelFormat, 1, _params._textureParams.flags);
    }
}

RenderTarget::~RenderTarget() {
    RenderTarget::FreeResource();
}

// TODO make static MakeRenderTarget for normal version and for a version that accepts a texture2D ?

bool RenderTarget::Initialize() {
    bool bWasInitialized = _texture && _texture->Initialize(_renderContext.get());
    return bWasInitialized;
}

void RenderTarget::CreateResource() {
    if( _texture) {
        _texture->CreateResource();
    }
}

void RenderTarget::FreeResource() {
    if(_renderContext && IsValidResource()) {
        _texture->FreeResource();
    }
}

void RenderTarget::SetTextureResource(void* resource) {
    if(_texture) {
        if(!_texture->GetResource()) {
            _texture->CreateResource();
        }
        
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
