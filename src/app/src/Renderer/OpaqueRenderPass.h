#pragma once
#include "RenderPass.h"
#include "RenderGraph/GraphBuilder.h"

DECLARE_SHADER(OpaqueRenderVertexShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_vs.spv)", ShaderType::VS)
    DECLARE_PARAMETER(glm::mat4, transform_matrix)
END_SHADER_DECLARATION()

DECLARE_SHADER(OpaqueRenderPixelShader, R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_fs.spv)", PS)
END_SHADER_DECLARATION()

DECLARE_PASS_DESC(OpaquePassDesc, OpaqueRenderVertexShader, OpaqueRenderPixelShader, OpaqueRenderPass)
    DECLARE_PARAMETER(std::string, scene_color)
    DECLARE_PARAMETER(std::string, scene_depth)
END_PASS_DESC_DECLARATION()

class OpaqueRenderPass : public IRenderPass {
public:
    OpaqueRenderPass(RenderGraph* render_graph, OpaquePassDesc pass_desc, std::string parent_graph_identifier);
    bool CreateCachedPSO() override;
    bool CreateFramebuffer() override;
    bool CreateCommandBuffer() override;
    bool RecordCommandBuffer() override;
    std::vector<VkCommandBuffer> GetCommandBuffers() override;
private:
    VkRenderPass CreateRenderPass();
    VkPipelineLayout CreatePipelineLayout(std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages);
    VkPipeline CreatePipeline(VkRenderPass render_pass, VkPipelineLayout pipeline_layout, const std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages);
    OpaquePassDesc pass_desc_;
    PipelineStateObject* pso_;
    PassResource* pass_resource_ = nullptr;
    std::string parent_graph_identifier_;
    RenderGraph* render_graph_;

    // Temp
    IndexRenderingData triangle_rendering_data_;
};