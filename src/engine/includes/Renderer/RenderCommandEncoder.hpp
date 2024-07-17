#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Components/PrimitiveProxyComponent.hpp"

class GraphicsContext;
class GraphicsPipeline;
class RenderContext;
class Shader;
class CommandBuffer;
class TextureResource;

class RenderCommandEncoder {
public:
    virtual ~RenderCommandEncoder() = default;
    
    static std::unique_ptr<RenderCommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);

    virtual void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) = 0;
    virtual void EndRenderPass() = 0;
    virtual void SetViewport(const glm::vec2& viewportSize) = 0;
    virtual void SetScissor(const glm::vec2& extent, const glm::vec2& offset) = 0;
    // Lets try to favor unfiroms, since push constants are not supported in other APIs like WebGPU
    virtual void UpdatePushConstants(GraphicsPipeline* graphicsPipeline, Shader* shader, const void* data) = 0;
    virtual void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) = 0;
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
//    virtual void ExecuteMemoryTransfer(Buffer* buffer) override;

protected:
    std::shared_ptr<RenderContext> _renderContext;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;
};
