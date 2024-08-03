#include "Renderer/Buffer.hpp"
#include "Renderer/render_context.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKImageBuffer.hpp"
#endif

std::shared_ptr<Buffer> Buffer::Create(RenderContext* renderContext) {
#ifdef USING_VULKAN_API
    auto buffer = std::make_shared<VKBuffer>();
    buffer->_renderContext = renderContext;
    
    return buffer;
#endif
    
    return nullptr;
}

std::shared_ptr<Buffer> Buffer::Create(RenderContext* renderContext, std::weak_ptr<TextureResource> resource) {
#ifdef USING_VULKAN_API
    auto buffer = std::make_shared<VKImageBuffer>(renderContext, resource);
    return buffer;
#endif
    
    return nullptr;
}

void Buffer::MarkDirty() {
    _isDirt = true;
}

void Buffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    _type = type;
    _usage = usage;
    _size = allocSize;
};
