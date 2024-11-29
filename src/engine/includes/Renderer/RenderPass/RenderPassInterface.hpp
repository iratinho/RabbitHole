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


/**
 *
 *   Motivation: I want to simplify the creation of push constants and uniform buffers, i also want to handle
 *  the situation where im trying to use push constants but the current API does not supports it or the amount of push
 *  constants is to high for the current device, in such cases we should pack all the data into a uniform buffer.
 *
 *   We have 2 stages, the compilation and the runtime stages, during compilation we will define the structure of our data
 *  and during runtime we will actually populate the data.
 *
 *   ShaderDataBlock is a data structure that defines a block of data, for example, push constants will be multiple
 *  sequential ShaderDataBlock's. Currently, we have a function that CollectsPushConstants, now we should have one
 *  called CollectShaderDataBlocks.
 *
 *  struct ShaderDataBlock {
 *      DataType _type;
 *      size_t _size;
 *      Proc _proc;
 *  }
 *
 *  The proc, is a member function pointer that allow us to bind a function pointer to allow us fetching the data at runtime
 *  this will make everything more automatic because we dont need to deal with that during processing.
 *
 *   In vulkan, during shader compilation we need to provide push constant configuration and uniform buffers configuration
 *  so if we have such generic behaviour, we need to be able to decide when are we trying to use one or another, a good way
 *  is to do the following:
 *     1 - Check if backend supports Push constants
 *     2 - Check the push constants size limit
 *
 *     If one of these checks fails that we are in a situation where we want to use uniform buffers instead.
 *
 *   I also suggest the creation of another data structure called ShaderDataStream, this allow us to have adition information
 *  about the blocks in general.
 *
 *  struct ShaderDataStream {
 *      std::vector<ShaderDataBlock> _blocks;
 *  }
 *
 *
 *
 */
class RenderPass {
protected:
    using EnttType = entt::entity;
    
public:
    virtual ~RenderPass() = default;
            
    [[nodiscard]] GraphicsPipeline* GetGraphicsPipeline() { return _pipeline.get(); };

    void EnqueueRendering(GraphBuilder* graphBuilder, Scene* scene);
    
public:
    // We could do all of it inside the build pipeline
    // RenderPass -> Pipeline -> Shaders (for the current pipeline)
    // Since this is generic code, we could actually handle it here pass
    virtual void Initialize(GraphicsContext* graphicsContext);
    
    [[nodiscard]] virtual std::string GetIdentifier() = 0;
            
    [[nodiscard]] virtual RenderAttachments GetRenderAttachments(GraphicsContext* graphicsContext) = 0;
    
    // TODO Rename to Rasterization params
    [[nodiscard]] virtual GraphicsPipelineParams GetPipelineParams() = 0;
    
    [[nodiscard]] virtual ShaderInputBindings CollectShaderInputBindings() = 0;
            
    [[nodiscard]] virtual std::vector<ShaderDataStream> CollectShaderDataStreams() = 0;
    
    [[nodiscard]] virtual std::vector<ShaderDataStream> GetPopulatedShaderDataStreams(GraphicsContext *graphicsContext, Scene *scene) { return {}; };
        
    [[nodiscard]] virtual std::string GetVertexShaderPath() = 0;
    
    [[nodiscard]] virtual std::string GetFragmentShaderPath() = 0;
    
    // Returns all textures resources that are going to be consumed during this pass
    // TODO we need to improve this in a way that we only load textures that we know that are going to be used
    [[nodiscard]] virtual std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene* scene) = 0;
    
    // Keep in mind that the buffers should already have the data, this will be called when trying to upload to gpu memory
    [[nodiscard]] virtual std::set<std::shared_ptr<Buffer>> GetBufferResources(Scene* scene) = 0;
    
    virtual void Process(GraphicsContext* graphicsContext, Encoders encoders, Scene* scene, GraphicsPipeline* pipeline) = 0;
    
    virtual void BindPushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* pipeline, RenderCommandEncoder* encoder, Scene* scene, EnttType entity, unsigned int entityIdx) {};
    
    virtual void BindShaderResources(GraphicsContext* graphicsContext, RenderCommandEncoder* encoder, Scene* scene, EnttType entity) {};
            
protected:
    std::shared_ptr<GraphicsPipeline> _pipeline;
    GraphicsContext* _graphicsContext = nullptr;
};

//class RenderPassExecuter {}; // Worth it for SOLID principles?

