#pragma once

#include "Renderer/RenderPass/RenderPassInterface.hpp"

class GraphicsContext;
class GraphBuilder;
class Scene;

class PhongRenderPass : public RenderPass {
public:
    std::string GetIdentifier() override { return "PhongRenderPass"; };
        
    RenderAttachments GetRenderAttachments(GraphicsContext *graphicsContext) override;
    
    GraphicsPipelineParams GetPipelineParams() override;
        
    void BindPushConstants(GraphicsContext *graphicsContext, GraphicsPipeline *pipeline, RenderCommandEncoder *encoder, Scene *scene, EnttType entity) override;
    
    ShaderInputBindings CollectShaderInputBindings() override;
    
    std::vector<PushConstant> CollectPushConstants() override;
    
    std::vector<ShaderResourceBinding> CollectResourceBindings() override;
    
    std::string GetVertexShaderPath() override;
    
    std::string GetFragmentShaderPath() override;
        
    std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene* scene) override;
    
    void Process(RenderCommandEncoder *encoder, Scene* scene, GraphicsPipeline* pipeline) override;
    
};
