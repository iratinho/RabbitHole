#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"

class GraphicsContext;
class GraphicsPipeline;
class Device;
class Shader;
class CommandBuffer;
class TextureResource;

// TODO Rename the CommandEncoder
class RenderCommandEncoder {
public:
    RenderCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device)
        : _commandBuffer(commandBuffer)
        , _graphicsContext(graphicsContext)
    {}

    virtual ~RenderCommandEncoder() = default;
    
    static std::unique_ptr<RenderCommandEncoder> MakeCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device);

    virtual void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) = 0;
    virtual void EndRenderPass() = 0;
    virtual void SetViewport(const glm::vec2& viewportSize) = 0;
    virtual void SetScissor(const glm::vec2& extent, const glm::vec2& offset) = 0;
    virtual void DispatchDataStreams(GraphicsPipeline* graphicsPipeline, const std::vector<ShaderDataStream> dataStreams) = 0;
    virtual void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) = 0;
    virtual void Draw(std::uint32_t count) = 0;
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
    virtual void UploadBuffer(std::shared_ptr<Buffer> buffer) = 0;
    virtual void UploadImageBuffer(std::shared_ptr<Texture2D> texture) = 0;

    [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return _graphicsContext; }

public:
    CommandBuffer* _commandBuffer = nullptr;
    GraphicsContext* _graphicsContext = nullptr;
};
