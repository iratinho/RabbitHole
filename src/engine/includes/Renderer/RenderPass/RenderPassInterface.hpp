#pragma once
#include "entt/entity/entity.hpp"
#include "Renderer/GPUDefinitions.h"
#include <set>

struct GraphicsPipelineParams;
class RenderCommandEncoder;
class GraphicsPipeline;
class GraphicsContext;
class GraphBuilder;
class Scene;
class PrimitiveProxyComponent;

#define REGISTER_RENDER_PASS(PassType) \
    namespace { \
        static PassType PassType; \
        struct PassType##Registrar { \
            PassType##Registrar() { \
                RenderSystemV2::RegisterRenderPass(&PassType); \
            } \
        }; \
        static PassType##Registrar global_##PassType##Registrar; \
    }

#ifndef STR_EXPAND
#define STR_EXPAND(tok) #tok
#endif

#ifndef STR
#define STR(tok) STR_EXPAND(tok)
#endif

#ifndef COMBINE_SHADER_DIR
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_DIR) "/" STR(name)
#endif

class RenderPass {
protected:
    using EnttType = entt::entity;
    
public:
    virtual ~RenderPass() = default;
            
public:
    // We could do all of it inside the build pipeline
    // RenderPass -> Pipeline -> Shaders (for the current pipeline)
    // Since this is generic code, we could actually handle it here pass
    virtual void Initialize(GraphicsContext* graphicsContext);

    [[nodiscard]] GraphicsPipeline* GetGraphicsPipeline() { return _pipeline.get(); };

public:
    [[nodiscard]] virtual std::string GetIdentifier() = 0;
            
    [[nodiscard]] virtual RenderAttachments GetRenderAttachments(GraphicsContext* graphicsContext) = 0;
    
    // TODO Rename to Rasterization params
    [[nodiscard]] virtual GraphicsPipelineParams GetPipelineParams() = 0;
    
    [[nodiscard]] virtual ShaderInputBindings CollectShaderInputBindings() = 0;
    
    [[nodiscard]] virtual std::vector<PushConstant> CollectPushConstants() = 0;
    
    [[nodiscard]] virtual std::vector<ShaderResourceBinding> CollectResourceBindings() = 0;
    
    [[nodiscard]] virtual std::string GetVertexShaderPath() = 0;
    
    [[nodiscard]] virtual std::string GetFragmentShaderPath() = 0;
    
    // Returns all textures resources that are going to be consumed during this pass
    // TODO we need to improve this in a way that we only load textures that we know that are going to be used
    [[nodiscard]] virtual std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene* scene) = 0;
    
    virtual void Process(Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) = 0;
    
    virtual void BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {};
    
    virtual void BindShaderResources(GraphicsContext* graphicsContext, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {};
            
protected:
    std::shared_ptr<GraphicsPipeline> _pipeline;
};

//class RenderPassExecuter {}; // Worth it for SOLID principles?

