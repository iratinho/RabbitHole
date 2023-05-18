#pragma once
#include "Renderer/RenderPass/RenderPass.h"
#include "Renderer/RenderGraph/GraphBuilder.h"

DECLARE_SHADER(FloorGridRenderVertexShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\floor_grid_vs.spv)", ShaderType::VS)
    DECLARE_PARAMETER(glm::mat4, transform_matrix)
END_SHADER_DECLARATION()

DECLARE_SHADER(FloorGridRenderPixelShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\floor_grid_fs.spv)", ShaderType::PS)
END_SHADER_DECLARATION()

class FloorGridRenderPass;
DECLARE_PASS_DESC(FloorGridPassDesc, FloorGridRenderVertexShader, FloorGridRenderPixelShader, FloorGridRenderPass)
    DECLARE_PARAMETER(std::shared_ptr<RenderTarget>, scene_color)
    DECLARE_PARAMETER(std::shared_ptr<RenderTarget>, scene_depth)
    DECLARE_PARAMETER(std::function<VkCommandBuffer()>, commandBuffer)
    DECLARE_PARAMETER(glm::mat4, projectionMatrix)
    DECLARE_PARAMETER(glm::mat4, viewMatrix)
END_PASS_DESC_DECLARATION()

class FloorGridRenderPass : public IRenderPass
{
public:
    FloorGridRenderPass(RenderGraph* render_graph, FloorGridPassDesc* pass_desc, std::string parent_graph_identifier);
    ~FloorGridRenderPass() override = default;
    bool Initialize() override;
    bool CreateFramebuffer() override;
    bool CreateCommandBuffer() override;
    bool RecordCommandBuffer() override;
    std::vector<VkCommandBuffer> GetCommandBuffers() override;

private:
    VkRenderPass CreateRenderPass() const;
    VkPipelineLayout CreatePipelineLayout(std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages);
    VkPipeline CreatePipeline(VkRenderPass render_pass, VkPipelineLayout pipeline_layout, const std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages);
    
    FloorGridPassDesc* pass_desc_;
    PipelineStateObject* _pso;
    std::string _parentGraphIdentifier;
    RenderGraph* _renderGraph;
};
