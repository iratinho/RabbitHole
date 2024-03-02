#include "Renderer/Texture2D.hpp"

#include "Renderer/TextureResource.hpp"
#include "Renderer/TextureView.hpp"

Texture2D::Texture2D(std::shared_ptr<RenderContext> renderContext)
    : _renderContext(renderContext) {
}

Texture2D::~Texture2D() {
    FreeResource();
}

bool Texture2D::Initialize(std::uint32_t width, std::uint32_t height, Format pixelFormat, unsigned int levels, TextureFlags flags, bool bExternalResource) {
    _width = width;
    _height = height;
    _pixelFormat = pixelFormat;
    _flags = flags;
    
    _textureResource = TextureResource::MakeResource(_renderContext.get(), this);
    
    if(!bExternalResource) {
        CreateResource();
    }
    
    return true;
}

TextureView* Texture2D::MakeTextureView() {
    return MakeTextureView(GetPixelFormat(), Range(0, 1));
}

TextureView* Texture2D::MakeTextureView(Format format, const Range &levels) { // Returns shared ptr or raw pointer??? What happens if we lose the texture and then the resource while the resource is being used in the gpu?? How to handle this type of things? Should i instead have a barrier that waits until the image is done processing???
    std::size_t hash = hash_value(format, levels);
    if(!_textureViews.contains(hash)) {
        std::shared_ptr<TextureView> textureView = TextureView::MakeTextureView(_renderContext.get(), _textureResource);
        textureView->CreateView(format, levels, TextureType::Texture_2D);
        _textureViews.emplace(hash, textureView);
    }
    
    return _textureViews[hash].get();
}

void Texture2D::CreateResource(void* resourceHandle) {
    if(_textureResource) {
        _textureResource->CreateResource();
    }
}

void Texture2D::FreeResource() {
    if(_textureView && _textureView.use_count() == 1) {
        _textureView.reset();
    }
    
    if(_textureResource && _textureResource.use_count() == 1) {
        _textureResource.reset();
    }
}

std::shared_ptr<TextureResource> Texture2D::GetResource() {
    return _textureResource;
}

std::uint32_t Texture2D::GetWidth() {
    return _width;
}

std::uint32_t Texture2D::GetHeight() {
    return _height;
}

Format Texture2D::GetPixelFormat() {
    return _pixelFormat;
}

TextureFlags Texture2D::GetTextureFlags() {
    return _flags;
}

void Texture2D::SetWrapModes(TextureWrapMode wrapU, TextureWrapMode wrapV, TextureWrapMode wrapW) {
    auto& [U, V, W] = _wrapModes;
    U = wrapU;
    V = wrapV;
    W = wrapW;
}

const std::tuple<TextureWrapMode, TextureWrapMode, TextureWrapMode>& Texture2D::GetWrapModes() {
    return _wrapModes;
}
