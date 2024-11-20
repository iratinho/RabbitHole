#include "Renderer/Buffer.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKImageBuffer.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureBuffer.hpp"
#endif

std::shared_ptr<Buffer> Buffer::Create(Device* device) {
#ifdef VULKAN_BACKEND
    auto buffer = std::make_shared<VKBuffer>();
    buffer->_device = device;
    
    return buffer;
#else
    auto buffer = std::make_shared<WebGPUBuffer>();
    buffer->_device = device;
    
    return buffer;
#endif
    
    return nullptr;
}

std::shared_ptr<Buffer> Buffer::Create(Device* device, std::weak_ptr<TextureResource> resource) {
#ifdef VULKAN_BACKEND
    auto buffer = std::make_shared<VKImageBuffer>(device, resource);
    return buffer;
#else
    auto buffer = std::make_shared<WebGPUTextureBuffer>(device, resource);
    return buffer;
#endif
    
    return nullptr;
}

void Buffer::MarkDirty() {
    _isDirt = true;
}

void Buffer::ClearDirty() {
    _isDirt = false;
}

void Buffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    _type = type;
    _usage = usage;
    _size = allocSize;
    
    MarkDirty();
};
