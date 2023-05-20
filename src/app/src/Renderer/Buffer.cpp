#include "Renderer/Buffer.hpp"
#include "Renderer/CommandBuffer.hpp"

namespace {
    VkBufferUsageFlags TranslateUsageFlags(EBufferUsage usage) {
        if(usage == EBufferUsage::BU_Geometry)
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
    }
}

Buffer::Buffer(RenderContext* renderContext)
    : _renderContext(renderContext)
    , _buffer(nullptr)
    , _memory(nullptr)
{
}

Buffer::~Buffer()
{
}

bool Buffer::AllocateBuffer(size_t allocationSize, EBufferUsage usage, bool bIsStagingBuffer) {
    if(!_renderContext) {
        return false;
    }

    
    VkBufferUsageFlags usageFlags = TranslateUsageFlags(usage);
    VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    
    if(bIsStagingBuffer) {
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        memoryFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    else {
        usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }
    
    VkBuffer buffer;
    VkDeviceMemory memory;
    _allocationSize = allocationSize;
    bool bOk = _renderContext->AllocateBuffer(allocationSize, buffer, memory, usageFlags, memoryFlags);

    _buffer = buffer;
    _memory = memory;
    
    return bOk;
}

bool Buffer::Upload(CommandBuffer* commandBuffer) {
    if(!commandBuffer && !commandBuffer->GetResource()) {
        return false;
    }

    // Create GPU buffer if it does not exist at this point
    if(!_buffer) {
        AllocateBuffer(_allocationSize, EBufferUsage::BU_Geometry, false);
    }

    _renderContext->CopyBuffer(static_cast<VkCommandBuffer>(commandBuffer->GetResource()), _stagingBuffer->_buffer, _buffer, _allocationSize);
    return true;
}

void* Buffer::Lock() {
    return _renderContext->LockBuffer(_memory, _allocationSize);
}

void Buffer::Unlock() const {
    _renderContext->UnlockBuffer(_memory);
}

void* Buffer::GetResource() {
    return _buffer;
}
