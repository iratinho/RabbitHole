#pragma once
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class GraphicsContext;
class RenderContext;

class VKRenderCommandEncoder : public RenderCommandEncoder {
public:
    VKRenderCommandEncoder(CommandBuffer* commandBuffer,GraphicsContext* graphicsContext, RenderContext* renderContext)
        : RenderCommandEncoder(commandBuffer, graphicsContext, renderContext)
    {}

    void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) override;
    void EndRenderPass() override;
    void SetViewport(const glm::vec2& viewportSize) override;
    void SetScissor(const glm::vec2& extent, const glm::vec2& offset) override;
    void UpdatePushConstants(GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) override;
    void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) override;
    
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
    void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) override;


//    void ExecuteMemoryTransfer(Buffer* buffer) override;
};
