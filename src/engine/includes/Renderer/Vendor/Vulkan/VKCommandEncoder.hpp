#pragma once
#include "Renderer/RenderCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKCommandEncoder : public RenderCommandEncoder {
public:    
    void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) override;
    void EndRenderPass() override;
    void SetViewport(const glm::vec2& viewportSize) override;
    void SetScissor(const glm::vec2& extent, const glm::vec2& offset) override;
    void UpdatePushConstants(GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) override;
    void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) override;
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;

//    void ExecuteMemoryTransfer(Buffer* buffer) override;
};
