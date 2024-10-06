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
    // Lets try to favor unfiroms, since push constants are not supported in other APIs like WebGPU
    virtual void UpdatePushConstants(GraphicsPipeline* graphicsPipeline, Shader* shader, const void* data) = 0;
    virtual void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) = 0;
    virtual void Draw(std::uint32_t count) = 0;
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
    virtual void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) = 0;

    [[nodiscard]] GraphicsContext* GetGraphicsContext() const { return _graphicsContext; }

public:
    CommandBuffer* _commandBuffer = nullptr;
    GraphicsContext* _graphicsContext = nullptr;
};
