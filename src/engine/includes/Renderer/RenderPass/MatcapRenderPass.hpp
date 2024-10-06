#pragma once
#include "Renderer/RenderPass/RenderPassInterface.hpp"

class GraphicsContext;
class RenderCommandEncoder;
class GraphBuilder;
class GraphicsPipeline;
class Scene;
struct MatCapMaterialComponent;
struct GraphicsPipelineParams;

class MatcapRenderPass : public RenderPass {    
public:
    std::string GetIdentifier() override { return "MatCapRenderPass"; };
            
    RenderAttachments GetRenderAttachments(GraphicsContext* graphicsContext) override;
    
    GraphicsPipelineParams GetPipelineParams() override;
        
    ShaderInputBindings CollectShaderInputBindings() override;
    
    std::vector<PushConstant> CollectPushConstants() override;
    
    std::vector<ShaderResourceBinding> CollectResourceBindings() override;

    std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene* scene) override;
    
    void Process(Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) override;
    
protected:
    void BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) override;
    
    void BindShaderResources(GraphicsContext* graphicsContext, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) override;
    
    std::string GetVertexShaderPath() override;
    
    std::string GetFragmentShaderPath() override;
    
    
};
