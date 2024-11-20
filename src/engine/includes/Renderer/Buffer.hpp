#pragma once

class TextureResource;

enum EBufferType : unsigned int {
    BT_Undefined    = 0,
    BT_HOST         = (1 << 0), // CPU accessile buffer
    BT_LOCAL        = (1 << 1), // GPU only accessible buffer
};

enum EBufferUsage : unsigned int {
    BU_Undefined    = 0,
    BU_Transfer     = (1 << 0),
    BU_Geometry     = (1 << 1),
    BU_Texture      = (1 << 2),
    BU_Uniform      = (1 << 3)
};

class Device;

struct BufferCreateParams {
    Device* _renderContext;
    EBufferType _type;
    EBufferUsage _usage;
};

// This buffer class is generic in a way that its underlaying data might be images or general resources
class Buffer {
public:
    virtual ~Buffer() = default;

    /**
     * Creates a new buffer
     *
     */
    static std::shared_ptr<Buffer> Create(Device* device);

    /**
     * Creates a new buffer
     *
     */
    static std::shared_ptr<Buffer> Create(Device* device, std::weak_ptr<TextureResource> texture2D);

        
    /**
     * @brief Initialized a generic gpu buffer
     */
    virtual void Initialize(EBufferType type, EBufferUsage usage, size_t allocSize);
        
    /**
     *
     */
    virtual void* LockBuffer() = 0;
    
    /**
     *
     */
    virtual void UnlockBuffer() = 0;
    
    // Dirty will force this buffer to be uploaded to the gpu
    void MarkDirty();
    
    void ClearDirty();
    
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
    Device* _device = nullptr;
    EBufferType _type = EBufferType::BT_Undefined;
    EBufferUsage _usage = EBufferUsage::BU_Undefined;
    size_t _size = 0;
};
