#pragma once
#include "Core/Utils.hpp"
#include "GPUDefinitions.h"


struct GraphicsPipelineParams;
class TextureResource;
class Texture2D;
class MaterialComponent;
class GraphicsContext;

enum class EGraphPassType {
    None,
    Raster,
    Blit
};

struct BaseNodeContext {
    
};

struct RasterNodeContext {
    RasterPassTarget _input;
    RenderAttachments _renderAttachments;
    
    PassResources _readResources;
    PassResources _writeResources;
    
    CommandCallback _callback;
    class GraphicsPipeline* _pipeline;
    std::string _passName;
};

struct BlitNodeContext {
    std::string _passName;
    BlitCommandCallback _callback;
    PassResources _readResources;
    PassResources _writeResources;
};

class RenderGraphNode {
public:
    EGraphPassType GetType() {
        if(std::holds_alternative<RasterNodeContext>(_ctx)) {
            return EGraphPassType::Raster;
        }
        
        if(std::holds_alternative<BlitNodeContext>(_ctx)) {
            return EGraphPassType::Blit;
        }

        assert(0);
        return EGraphPassType::None;
    };
    
    template <typename ContextType>
    ContextType& GetContext() { return std::get<ContextType>(_ctx); };
    
    static PassResources* GetReadResources(RenderGraphNode* node) {
        if(!node) {
            assert(0 && "GetWriteResources called with invalid node parameter.");
        }

        if(node->GetType() == EGraphPassType::Raster) {
            return &node->GetContext<RasterNodeContext>()._readResources;
        }
        
        if(node->GetType() == EGraphPassType::Blit) {
            return &node->GetContext<BlitNodeContext>()._readResources;
        }
    };
    
    static PassResources* GetWriteResources(RenderGraphNode* node) {
        if(!node) {
            assert(0 && "GetWriteResources called with invalid node parameter.");
        }
        
        if(node->GetType() == EGraphPassType::Raster) {
            return &node->GetContext<RasterNodeContext>()._writeResources;
        }
        
        if(node->GetType() == EGraphPassType::Blit) {
            return &node->GetContext<BlitNodeContext>()._writeResources;
        }
    };

            
    bool DependesOn(RenderGraphNode& node) {
        std::vector<std::shared_ptr<Texture2D>> readTextures;
        std::vector<std::shared_ptr<Texture2D>> writeTextures;
        
        std::vector<std::shared_ptr<Buffer>> readBufferResources;
        std::vector<std::shared_ptr<Buffer>> writeBufferResources;
        
        if(PassResources* readResources = GetReadResources(this)) {
            readTextures.insert(readTextures.end(), readResources->_textures.begin(), readResources->_textures.end());
            readBufferResources.insert(readBufferResources.end(), readResources->_buffersResources.begin(), readResources->_buffersResources.begin());
        }
        
        if(PassResources* writeResources = GetWriteResources(&node)) {
            writeTextures.insert(writeTextures.end(), writeResources->_textures.begin(), writeResources->_textures.end());
            writeBufferResources.insert(writeBufferResources.end(), writeResources->_buffersResources.begin(), writeResources->_buffersResources.end());
        }
        
        bool bFoundDependency = false;
        for (auto readTexture : readTextures) {
            bFoundDependency |= std::find(writeTextures.begin(), writeTextures.end(), readTexture) != writeTextures.end();
        }
        
        for (auto readBufferResource : readBufferResources) {
            bFoundDependency |= std::find(writeBufferResources.begin(), writeBufferResources.end(), readBufferResource) != writeBufferResources.end();
        }
        
        return bFoundDependency;
    }

protected:
    EGraphPassType _graphPassType;
    std::variant<RasterNodeContext, BlitNodeContext> _ctx;
    
    friend class GraphBuilder;
};

template <typename T>
concept AcceptRasterPassIf = std::derived_from<T, MaterialComponent>;

class GraphBuilder {
public:
    GraphBuilder() {};
    GraphBuilder(GraphicsContext* graphicsContext);
    
    template <typename T> requires(AcceptRasterPassIf<T>)
    void AddRasterPass(const std::string& passName, Scene* scene, const GraphicsPipelineParams& pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback&& callback);
    
    template <typename T> requires(!AcceptRasterPassIf<T>)
    void AddRasterPass(const std::string&, Scene*, const GraphicsPipelineParams&, const RenderAttachments&, const CommandCallback&&) {
        static_assert(AcceptRasterPassIf<T>, "AddRaster pass function being called with wrong material component template parameter");
    };

    void AddBlitPass(std::string passName, PassResources resources, const BlitCommandCallback &&callback);

    void AddBlitPass(std::string passName, PassResources readResources, PassResources writeResources, const BlitCommandCallback &&callback);
            
    void Exectue(std::function<void(RenderGraphNode)> func);
        
private:
    void MakeImplicitBlitTransfer(const PassResources& passResources);
    
protected:
    std::vector<RenderGraphNode> _nodes;

private:
    DirectGraph<RenderGraphNode> _graph;
    GraphicsContext* _graphicsContext;
    DAG _dag;
};
