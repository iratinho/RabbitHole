#pragma once
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class GraphicsContext;
class Device;

class VKRenderCommandEncoder : public RenderCommandEncoder {
public:
    VKRenderCommandEncoder(CommandBuffer* commandBuffer,GraphicsContext* graphicsContext, Device* device)
        : RenderCommandEncoder(commandBuffer, graphicsContext, device)
    {}

    void BeginRenderPass(GraphicsPipeline* pipeline, const RenderAttachments& attachments) override;
    void EndRenderPass() override;
    void SetViewport(const glm::vec2& viewportSize) override;
    void SetScissor(const glm::vec2& extent, const glm::vec2& offset) override;
    void DispatchDataStreams(GraphicsPipeline* graphicsPipeline, const std::vector<ShaderDataStream> dataStreams) override;
    void DrawPrimitiveIndexed(const PrimitiveProxyComponent& proxy) override;
    void Draw(std::uint32_t count) override;
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
    void UploadImageBuffer(std::shared_ptr<Texture2D> texture) override;



//    void ExecuteMemoryTransfer(Buffer* buffer) override;
};
