#include "Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
#include "Renderer/Texture2D.hpp"

void WebGPUTextureResource::CreateResource() {
    // We do not need to create a buffer because this resource is externally managed
    if(_bIsExternalResource) {
        return;
    }
        
    WebGPUDevice* device = (WebGPUDevice*)_device;
    if(!device) {
        assert(false && "Invalid device in WebGPUImageBuffer::Initialize");
        return;
    }
    
    WGPUDevice wgpuDevice = device->GetWebGPUDevice();
    if(!wgpuDevice) {
        assert(false && "Invalid native device in WebGPUImageBuffer::Initialize");
        return;
    }
    
    std::array<WGPUTextureFormat, 1> viewFormats = {TranslateFormat(_texture->GetPixelFormat())};
        
    WGPUTextureDescriptor textureDescriptor {};
    textureDescriptor.dimension = WGPUTextureDimension::WGPUTextureDimension_2D;
    textureDescriptor.size = {_texture->GetWidth(), _texture->GetHeight(), 1};
    textureDescriptor.format = TranslateFormat(_texture->GetPixelFormat());
    textureDescriptor.label = "give a label..";
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.sampleCount = 1;
    textureDescriptor.usage = TranslateTextureUsageFlags(_texture->GetTextureFlags());
    textureDescriptor.viewFormats = viewFormats.data();
    textureDescriptor.viewFormatCount = viewFormats.size();
    _nativeTexture = wgpuDeviceCreateTexture(wgpuDevice, &textureDescriptor);
    
    if(!_nativeTexture) {
        assert(false && "Unable to create native texture in WebGPUImageBuffer::Initialize");
        return;
    }
    
    // only allocate pixel data buffer for sampled images
    if(_texture->GetTextureFlags() & TextureFlags::Tex_SAMPLED_OP) {
        _buffer = Buffer::Create(_device, _texture->GetResource());
        
        if(_buffer) {
            _buffer->Initialize(EBufferType::BT_HOST, EBufferUsage::BU_Texture, _texture->GetImageDataSize());
        }
    }
}

void WebGPUTextureResource::SetExternalResource(void *handle) {
    if(!handle) {
        return;
    }

    _nativeTexture  = static_cast<WGPUTexture>(handle);
}

void WebGPUTextureResource::FreeResource() {
    if(!_bIsExternalResource) {
        wgpuTextureRelease(_nativeTexture);
    }
}

bool WebGPUTextureResource::HasValidResource() {
    return _nativeTexture != nullptr;
}

void* WebGPUTextureResource::Lock() {
    if(!_buffer) {
        assert(0);
        return nullptr;
    }
    
    return _buffer->LockBuffer();
}

void WebGPUTextureResource::Unlock() {
    if(!_buffer) {
        assert(0);
        return;
    }
    
    _buffer->UnlockBuffer();
}
