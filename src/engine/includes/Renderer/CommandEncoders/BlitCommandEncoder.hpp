#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"

class RenderContext;
class CommandBuffer;
class Buffer;
class Texture2D;

typedef GeneralCommandEncoder Super;

class BlitCommandEncoder : public GeneralCommandEncoder {
public:
    
    BlitCommandEncoder(CommandBuffer* commandBuffer,GraphicsContext* graphicsContext, RenderContext* renderContext)
        : GeneralCommandEncoder(commandBuffer, graphicsContext, renderContext)
    {}

    virtual ~BlitCommandEncoder() = default;
    
    static std::unique_ptr<BlitCommandEncoder> MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext);
    
    /// Transfers the buffer data into a GPU memory buffer
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    
    virtual void UploadImageBuffer(std::shared_ptr<Texture2D> texture) = 0;
};
