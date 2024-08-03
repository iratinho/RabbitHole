#include "Renderer/Texture2D.hpp"

#include "Renderer/TextureResource.hpp"
#include "Renderer/TextureView.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::~Texture2D() {
    FreeResource();
    
    if(_data) {
        stbi_image_free(_data);
    }
}

std::shared_ptr<Texture2D> Texture2D::MakeFromPath(const char* path, Format pixelFormat) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_path = path;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = (TextureFlags)(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_TRANSFER_DEST_OP);

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeFromExternalResource(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags, unsigned int levels) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = flags;
    texture2D->_hasExternalResource = true;

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeTexturePass(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags, unsigned int levels) {
    
    if(flags & TextureFlags::Tex_None) {
        return nullptr;
    }
    
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = flags;
    texture2D->_hasExternalResource = false;

    return texture2D;
}

// Should i remove this initialize function? It does not do much
bool Texture2D::Initialize(RenderContext* renderContext) {
    if(!renderContext) {
        return false;
    }
    
    _renderContext = renderContext;
        
    return true;
}

TextureView* Texture2D::MakeTextureView() {
    return MakeTextureView(GetPixelFormat(), Range(0, 1));
}

TextureView* Texture2D::MakeTextureView(Format format, const Range &levels) { // Returns shared ptr or raw pointer??? What happens if we lose the texture and then the resource while the resource is being used in the gpu?? How to handle this type of things? Should i instead have a barrier that waits until the image is done processing???
    std::size_t hash = hash_value(format, levels);
    if(!_textureViews.contains(hash)) {
        std::shared_ptr<TextureView> textureView = TextureView::MakeTextureView(_renderContext, _textureResource);
        textureView->CreateView(format, levels, TextureType::Texture_2D);
        _textureViews.emplace(hash, textureView);
    }
    
    return _textureViews[hash].get();
}

void Texture2D::CreateResource(void* resourceHandle) {
    if(!_textureResource) {
        _textureResource = TextureResource::MakeResource(_renderContext, this, _hasExternalResource);
    }
    
    _textureResource->CreateResource();
    
    if(resourceHandle && _hasExternalResource) {
        _textureResource->SetExternalResource(resourceHandle);
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
    _wrapU = wrapU;
    _wrapV = wrapV;
    _wrapW = wrapW;
}

void Texture2D::SetFilter(TextureFilter magnificationFilter, TextureFilter minificationFilter) {
    _magFilter = magnificationFilter;
    _minFilter = minificationFilter;
}

void Texture2D::Reload(bool bIsDeepReload) {
    if(_textureResource && !bIsDeepReload) {
        return _data;
    }
    
    if(strlen(_path) == 0) {
        assert(0 && "Texture2D::Reload() - No path was set for the texture");
        return;
    }

    // LEAK, This data do not exist
    if(_data) {
        stbi_image_free(reinterpret_cast<void*>(_data));
    }

    int x,y,n;
    void* data = stbi_load(_path, &x, &y, &n, 0);

    if(data == nullptr) {
        assert(0 && "Texture2D::Reload() - Failed to load image from file");
        return;
    }

    _width = x;
    _height = y;
    _dataSize = x * y * n * sizeof(unsigned char);
    
    FreeResource();
    
    // todo: need to improve this resources part.. i want to be able to create resource only for stagging and then for buffer
    // so we need to split the creation part
    // Creates the resource so that we can copy its pixel data
    CreateResource();
    
    void* buffer = _textureResource->Lock();
    std::memcpy(buffer, data, _dataSize);
    _textureResource->Unlock();
}
