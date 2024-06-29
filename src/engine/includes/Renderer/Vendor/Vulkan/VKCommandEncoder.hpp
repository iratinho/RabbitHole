#pragma once
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKCommandEncoder : public CommandEncoder {
public:    
    void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) override;
    void EndRenderPass() override;
    void SetViewport(GraphicsContext *graphicsContext, const glm::vec2& viewportSize) override;
    void SetScissor(const glm::vec2& extent, const glm::vec2& offset) override;
    void UpdatePushConstants(GraphicsContext *graphicsContext, GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) override;
    void DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy) override;
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;

//    void ExecuteMemoryTransfer(Buffer* buffer) override;
};
