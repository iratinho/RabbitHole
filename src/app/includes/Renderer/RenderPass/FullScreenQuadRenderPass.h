#pragma once
#include "Renderer/RenderPass/RenderPass.h"
#include "Renderer/RenderGraph/GraphBuilder.h"

DECLARE_SHADER(FullScreenQuadVertexShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\fullScreenQuadShader_vs.spv)", ShaderType::VS)
    DECLARE_PARAMETER(glm::mat4, transform_matrix)
END_SHADER_DECLARATION()

DECLARE_SHADER(FullScreenQuadPixelShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\fullScreenQuadShader_fs.spv)", ShaderType::PS)
END_SHADER_DECLARATION()

class FullScreenQuadRenderPass;
DECLARE_PASS_DESC(FullScreenQuadPassDesc, FullScreenQuadVertexShader, FullScreenQuadPixelShader, FullScreenQuadRenderPass)
    DECLARE_PARAMETER(std::shared_ptr<RenderTarget>, sceneColor)
    DECLARE_PARAMETER(std::shared_ptr<RenderTarget>, sceneDepth)
    DECLARE_PARAMETER(std::shared_ptr<RenderTarget>, texture)
END_PASS_DESC_DECLARATION()

class FullScreenQuadRenderPass : public IRenderPass
{
public:
    FullScreenQuadRenderPass(RenderGraph* renderGraph, FullScreenQuadPassDesc* passDesc,
                             std::string parentGraphIdentifier);

    bool Initialize() override;
    bool CreateFramebuffer() override;
    bool CreateCommandBuffer() override;
    bool RecordCommandBuffer() override;
    std::vector<VkCommandBuffer> GetCommandBuffers() override;

private:
    VkRenderPass CreateRenderPass() const;
    VkPipelineLayout CreatePipelineLayout(VkRenderPass renderPass, std::array<VkPipelineShaderStageCreateInfo, 2>& shaderStages) const;
    std::pair<VkPipeline, VkPipelineLayout> CreateGraphicsPipeline(VkRenderPass renderPass) const;
    
    RenderGraph* _renderGraph;
    FullScreenQuadPassDesc* _passDesc;
    PipelineStateObject* _pso;
    std::string _parentGraphIdentifier;
};