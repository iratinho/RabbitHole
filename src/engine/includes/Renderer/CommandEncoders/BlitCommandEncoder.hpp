#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"

class RenderContext;
class CommandBuffer;
class Buffer;
class Texture2D;

class BlitCommandEncoder : public virtual GeneralCommandEncoder {
public:
    virtual ~BlitCommandEncoder() = default;
    
    static std::unique_ptr<BlitCommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);
    
    /// Transfers the buffer data into a GPU memory buffer
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    
    virtual void UploadImageBuffer(std::shared_ptr<Texture2D> texture) = 0;
};
