#pragma once
#include "Renderer/render_context.h"

class RenderContext;
class CommandBuffer;

enum EBufferUsage {
    BU_Empty,
    BU_Geometry
};

class Buffer {
public:
    Buffer() = delete;
    Buffer(RenderContext* renderContext);
    ~Buffer();

    bool AllocateBuffer(size_t allocationSize, EBufferUsage usage, bool bIsStagingBuffer = true);
    bool Upload(CommandBuffer* commandBuffer);
    void* Lock();
    void Unlock() const;

    void* GetResource();
    
private:
    RenderContext* _renderContext;
    VkBuffer _buffer;
    size_t _allocationSize = 0;
    VkDeviceMemory _memory;

    std::unique_ptr<Buffer> _stagingBuffer;

    friend class BufferAction;
};
