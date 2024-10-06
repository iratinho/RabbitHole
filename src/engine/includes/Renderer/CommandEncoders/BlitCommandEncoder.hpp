#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"

class Device;
class CommandBuffer;
class Buffer;
class Texture2D;
class GraphicsContext;

class BlitCommandEncoder {
public:
    
    BlitCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device)
        : _commandBuffer(commandBuffer)
        , _graphicsContext(graphicsContext)
    {}

    virtual ~BlitCommandEncoder() = default;
    
    static std::unique_ptr<BlitCommandEncoder> MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device);
    /// Transfers the buffer data into a GPU memory buffer
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    virtual void UploadImageBuffer(std::shared_ptr<Texture2D> texture) = 0;

    [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return _graphicsContext; }

protected:
    CommandBuffer* _commandBuffer = nullptr;
    GraphicsContext* _graphicsContext = nullptr;
};
