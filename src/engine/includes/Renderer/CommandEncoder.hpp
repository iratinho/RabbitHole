#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Components/PrimitiveProxyComponent.hpp"

class GraphicsContext;
class GraphicsPipeline;
class RenderContext;
class Shader;
class CommandBuffer;
class TextureResource;

class CommandEncoder {
public:
    virtual ~CommandEncoder() = default;
    
    static std::unique_ptr<CommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);

    virtual void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) = 0;
    virtual void EndRenderPass() = 0;
    virtual void SetViewport(GraphicsContext *graphicsContext, const glm::vec2& viewportSize) = 0;
    virtual void SetScissor(const glm::vec2& extent, const glm::vec2& offset) = 0;
    // Lets try to favor unfiroms, since push constants are not supported in other APIs like WebGPU
    virtual void UpdatePushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* graphicsPipeline, Shader* shader, const void* data) = 0;
    virtual void DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy) = 0;
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
//    virtual void ExecuteMemoryTransfer(Buffer* buffer) override;

protected:
    std::shared_ptr<RenderContext> _renderContext;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;
};
