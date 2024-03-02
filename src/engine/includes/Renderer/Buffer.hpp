#pragma once

enum EBufferType : unsigned int {
    BT_Undefined    = 0,
    BT_HOST         = (1 << 0),
    BT_LOCAL        = (1 << 1),
};

enum EBufferUsage : unsigned int {
    BU_Undefined    = 0,
    BU_Transfer     = (1 << 0),
    BU_Geometry     = (1 << 1)
};

class RenderContext;

class Buffer {
public:
    
    /**
     * Creates a new buffer
     *
     */
    static std::shared_ptr<Buffer> Create(RenderContext* renderContext, EBufferType type, EBufferUsage usage, size_t allocSize);
    
    /**
     *
     */
    virtual void Initialize() = 0;
    
    /**
     *
     */
    virtual void* LockBuffer() = 0;
    
    /**
     *
     */
    virtual void UnlockBuffer() = 0;
    
    void MarkDirty();
    
    bool IsDirty() { return _isDirt; }
    
    /**
     * @returns the type of the buffer in terms of allocation locality
     */
    EBufferType GetType() { return _type; };
    
    /*
     * @returns the usage type of the buffer
     */
    EBufferUsage GetUsage() { return _usage; };
    
    /*
     * @returns the allocation size of the buffer
     */
    size_t GetSize() { return _size; };
    
protected:
    bool _isDirt = true;
    RenderContext* _renderContext = nullptr;
    EBufferType _type = EBufferType::BT_Undefined;
    EBufferUsage _usage = EBufferUsage::BU_Undefined;
    size_t _size = 0;
};










// __________________________________________ //

//#include "Renderer/render_context.hpp"
//
//class RenderContext;
//class CommandBuffer;
//
//enum class EBufferType {
//    GPU,
//    CPU
//};
//
//enum EBufferUsage {
//    BU_Empty,
//    BU_Geometry
//};
//
//class Buffer {
//public:
//    Buffer(RenderContext* renderContext);
//    ~Buffer();
//
//    bool AllocateBuffer(size_t allocationSize, EBufferUsage usage, bool bIsStagingBuffer = true);
//    bool Upload(CommandBuffer* commandBuffer);
//    void* Lock();
//    void Unlock() const;
//
//    void* GetResource();
//    
//private:
//    RenderContext* _renderContext;
//    VkBuffer _buffer;
//    size_t _allocationSize = 0;
//    VkDeviceMemory _memory;
//
//    std::unique_ptr<Buffer> _stagingBuffer;
//
//    friend class BufferAction;
//};
