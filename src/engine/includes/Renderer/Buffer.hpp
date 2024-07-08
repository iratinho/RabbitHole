#pragma once

class Texture2D;

enum EBufferType : unsigned int {
    BT_Undefined    = 0,
    BT_HOST         = (1 << 0),
    BT_LOCAL        = (1 << 1),
};

enum EBufferUsage : unsigned int {
    BU_Undefined    = 0,
    BU_Transfer     = (1 << 0),
    BU_Geometry     = (1 << 1),
    BU_Texture      = (1 << 2)
};

class RenderContext;

// This buffer class is generic in a way that its underlaying data might be images or general resources
class Buffer {
public:
    /**
     * Creates a new buffer
     *
     */
    static std::shared_ptr<Buffer> Create(RenderContext* renderContext);
        
    /**
     * @brief Initialized a generic gpu buffer
     */
    virtual void Initialize(EBufferType type, EBufferUsage usage, size_t allocSize);
    
    /**
     * @brief Initialize a texture2D buffer
     */
    virtual void InitializeFromTexture(EBufferType type, Texture2D* texture2D, size_t allocSize);
    
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
