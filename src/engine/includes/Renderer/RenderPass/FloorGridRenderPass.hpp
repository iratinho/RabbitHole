#pragma once
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/RenderSystemV2.hpp"

class GraphicsContext;
class GraphBuilder;
class Scene;

class FloorGridRenderPass : public RenderPass {
public:
    FloorGridRenderPass();
    
    std::string GetIdentifier() override { return "FloorGridRenderPass"; };
        
    RenderAttachments GetRenderAttachments(GraphicsContext *graphicsContext) override;
    
    GraphicsPipelineParams GetPipelineParams() override;
        
    ShaderInputBindings CollectShaderInputBindings() override;
        
    std::vector<ShaderDataStream> CollectShaderDataStreams() override;
    
    std::vector<ShaderDataStream> GetPopulatedShaderDataStreams(GraphicsContext *graphicsContext, Scene *scene) override;
            
    void Process(GraphicsContext* graphicsContext, Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) override;
    
    void BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity, unsigned int entityIdx) override;
    
    std::string GetVertexShaderPath() override;
    
    std::string GetFragmentShaderPath() override;
    
    std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene* scene) override;
    
    std::set<std::shared_ptr<Buffer>> GetBufferResources(Scene* scene) override;
    
private:
    std::shared_ptr<Buffer> _dataBuffer;
};
