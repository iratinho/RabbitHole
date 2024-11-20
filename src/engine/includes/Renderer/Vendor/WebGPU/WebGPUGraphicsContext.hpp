#pragma once
#include "Renderer/GraphicsContext.hpp"

class WebGPUGraphicsContext : public GraphicsContext {
public:
    WebGPUGraphicsContext(Device* device);
    ~WebGPUGraphicsContext() = default;

    bool Initialize() override;
    void BeginFrame() override;
    void EndFrame() override;

    std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> GetPipelines() override;
    std::shared_ptr<Texture2D> GetSwapChainColorTexture() override;
    std::shared_ptr<Texture2D> GetSwapChainDepthTexture() override;

    void Present() override;
    void Execute(RenderGraphNode node) override;

private:
    std::shared_ptr<CommandBuffer> _commandBuffer;
};
