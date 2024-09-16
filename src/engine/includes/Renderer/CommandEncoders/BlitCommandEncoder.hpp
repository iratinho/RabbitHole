#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"

class Device;
class CommandBuffer;
class Buffer;
class Texture2D;

typedef GeneralCommandEncoder Super;

class BlitCommandEncoder : public GeneralCommandEncoder {
public:
    
    BlitCommandEncoder(CommandBuffer* commandBuffer,GraphicsContext* graphicsContext, Device* device)
        : GeneralCommandEncoder(commandBuffer, graphicsContext, device)
    {}

    virtual ~BlitCommandEncoder() = default;
    
    static std::unique_ptr<BlitCommandEncoder> MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device);
    
    /// Transfers the buffer data into a GPU memory buffer
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    
    virtual void UploadImageBuffer(std::shared_ptr<Texture2D> texture) = 0;
};
