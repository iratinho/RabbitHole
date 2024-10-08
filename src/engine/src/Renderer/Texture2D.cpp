#include "Renderer/Texture2D.hpp"
#include "Renderer/TextureResource.hpp"
#include "Renderer/TextureView.hpp"
#include "Renderer/Device.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::~Texture2D() {
    Texture2D::FreeResource();
    
    if(_loadFlags == TexLoad_Path && _data) {
        stbi_image_free(_data);
    }

    if(_loadFlags ==  TexLoad_Data && _data) {
        delete _data;
        _data = nullptr;
    }
}

// Need to make this interface better....
// We need multiple flows
// 1. MakeFromPath | 2. MakeFromData | 3. MakeFromExternalResource | 4. MakeDynamicTexture

std::shared_ptr<Texture2D> Texture2D::MakeFromPath(const char* path, Format pixelFormat) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_path = path;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = static_cast<TextureFlags>(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_TRANSFER_DEST_OP);
    texture2D->_loadFlags = TexLoad_Path;

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeFromData(std::uint32_t width, std::uint32_t height, Format pixelFormat,
    const void *data, std::size_t size) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = static_cast<TextureFlags>(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_TRANSFER_DEST_OP);
    texture2D->_loadFlags = TexLoad_Data;
    texture2D->_dataSize = size;

    texture2D->_data = new unsigned char[size];
    std::memcpy(texture2D->_data, data, size);

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeFromExternalResource(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags, unsigned int levels) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = flags;
    texture2D->_loadFlags = TexLoad_ExternalResource;

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeAttachmentTexture(std::uint32_t width, std::uint32_t height,
    Format pixelFormat) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = TextureFlags::Tex_COLOR_ATTACHMENT;
    texture2D->_loadFlags = TexLoad_Attachment;

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeAttachmentDepthTexture(std::uint32_t width, std::uint32_t height) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = Format::FORMAT_D32_SFLOAT;
    texture2D->_flags = TextureFlags::Tex_DEPTH_ATTACHMENT;
    texture2D->_loadFlags = TexLoad_Attachment;

    return texture2D;
}

std::shared_ptr<Texture2D> Texture2D::MakeDynamicTexture(std::uint32_t width, std::uint32_t height, Format pixelFormat) {
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = static_cast<TextureFlags>(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_TRANSFER_DEST_OP);
    texture2D->_loadFlags = TexLoad_DynamicData;

    return texture2D;
}

// Make it return unique_ptr
std::shared_ptr<Texture2D> Texture2D::MakeTexturePass(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags, unsigned int levels) {
    
    if(flags & TextureFlags::Tex_None) {
        return nullptr;
    }
    
    auto texture2D = std::make_shared<Texture2D>();
    texture2D->_width = width;
    texture2D->_height = height;
    texture2D->_pixelFormat = pixelFormat;
    texture2D->_flags = static_cast<TextureFlags>(flags | TextureFlags::Tex_TRANSFER_DEST_OP);

    return texture2D;
}

// Should i remove this initialize function? It does not do much
bool Texture2D::Initialize(Device* device) {
    if(!device) {
        return false;
    }
    
    _device = device;
        
    return true;
}

TextureView* Texture2D::MakeTextureView() {
    return MakeTextureView(GetPixelFormat(), Range(0, 1));
}

TextureView* Texture2D::MakeTextureView(Format format, const Range &levels) { // Returns shared ptr or raw pointer??? What happens if we lose the texture and then the resource while the resource is being used in the gpu?? How to handle this type of things? Should i instead have a barrier that waits until the image is done processing???
    std::size_t hash = hash_value(format, levels);
    if(!_textureViews.contains(hash)) {
        std::shared_ptr<TextureView> textureView = TextureView::MakeTextureView(_device, _textureResource);
        textureView->CreateView(format, levels, TextureType::Texture_2D);
        _textureViews.emplace(hash, textureView);
    }
    
    return _textureViews[hash].get();
}

void Texture2D::CreateResource(void* resourceHandle) {
    if(_textureResource) {
        assert(0 && "Trying to create new texture resource while having one.");
        return;
    }

    if(!_textureResource) {
        _textureResource = TextureResource::MakeResource(_device, this, _loadFlags == TexLoad_ExternalResource);
    }
    
    _textureResource->CreateResource();
    
    if(resourceHandle && _loadFlags == TexLoad_ExternalResource) {
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

std::uint32_t Texture2D::GetWidth() const {
    return _width;
}

std::uint32_t Texture2D::GetHeight() const {
    return _height;
}

Format Texture2D::GetPixelFormat() const {
    return _pixelFormat;
}

TextureFlags Texture2D::GetTextureFlags() const {
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
    if(!IsDirty()) {
        return;
    }

    if(_loadFlags == TexLoad_DynamicData) {
        HandleDynamicDataReload();
        return;
    }

    if(_loadFlags == TexLoad_Path) {
        HandleFromPathReload();
        return;
    }

    if(_loadFlags == TexLoad_Data) {
        HandleFromDataReload();
        return;
    }
}

void Texture2D::MakeDirty() const {
    if(_textureResource) {
        _textureResource->MakeDirty();
    }
}

bool Texture2D::IsDirty() const {
    if(!_textureResource) {
        return true;
    }
        
    return _textureResource->IsDirty();
}

void Texture2D::HandleDynamicDataReload() {
    _dataSize = _width * _height * 4; // TODO create method to get the correct value based on format
    FreeResource();
    CreateResource(nullptr);

}

void Texture2D::HandleFromPathReload() {
    if(strlen(_path) == 0) {
        assert(0 && "Texture2D::HandleFromPathReload() - No path was set for the texture");
        return;
    }

    if(_data) {
        stbi_image_free(reinterpret_cast<void*>(_data));
    }

    int x,y,n;
    void* data = stbi_load(_path, &x, &y, &n, STBI_rgb_alpha);

    if(data == nullptr) {
        assert(0 && "Texture2D::Reload() - Failed to load image from file");
        return;
    }

    _width = x;
    _height = y;

    /*
     * A note about the 4, this is the number of channels we are reading the image from,
     * a current limitation of stb is that the 'n' will always be the number that it would
     * have been if you said 0, this is not ideal with jpg and png. We need a better way to
     * get the number of the channels based on the image type. and this also needs to match
     * the current texture format we are using
     */
    _dataSize = x * y * 4;

    FreeResource();
    CreateResource(nullptr);

    void* buffer = _textureResource->Lock();
    std::memcpy(buffer, data, _dataSize);
    _textureResource->Unlock();
}

void Texture2D::HandleFromDataReload() {
    if(!_data) {
        assert(0 && "Texture2D::HandleFromDataReload() - No pixel data");
        return;
    }

    FreeResource();
    CreateResource(nullptr);

    void* buffer = _textureResource->Lock();
    std::memcpy(buffer, _data, _dataSize);
    _textureResource->Unlock();
}
