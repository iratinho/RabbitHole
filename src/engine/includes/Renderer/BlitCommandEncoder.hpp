#pragma once

class RenderContext;
class CommandBuffer;
class Buffer;

class BlitCommandEncoder {
public:
    virtual ~BlitCommandEncoder() = default;
    
    static std::unique_ptr<BlitCommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);
    
    /// Transfers the buffer data into a GPU memory buffer
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    
protected:
    std::shared_ptr<RenderContext> _renderContext;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;

};
