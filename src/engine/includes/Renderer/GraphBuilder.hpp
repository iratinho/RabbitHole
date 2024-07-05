#pragma once
#include "Core/Utils.hpp"
#include "GPUDefinitions.h"
#include "GraphicsPipeline.hpp"

struct GraphicsPipelineParams;
class TextureResource;
class MaterialComponent;

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
            
    bool DependesOn(RenderGraphNode& node) {
        
        std::vector<std::shared_ptr<TextureResource>> readTextureResources;
        std::vector<std::shared_ptr<TextureResource>> writeTextureResources;
        
        std::vector<std::shared_ptr<Buffer>> readBufferResources;
        std::vector<std::shared_ptr<Buffer>> writeBufferResources;

        // We can make this dependency walking more generic... 
        
        if(this->GetType() == EGraphPassType::Raster) {
            const RasterNodeContext& currentNodeContext = this->GetContext<RasterNodeContext>();
            
            // Texture Resources
            readTextureResources.insert(readTextureResources.end(), currentNodeContext._readResources._textureResources.begin(), currentNodeContext._readResources._textureResources.end());
            
            // Buffer Resources
            readBufferResources.insert(readBufferResources.end(), currentNodeContext._readResources._buffersResources.begin(), currentNodeContext._readResources._buffersResources.begin());
        }
        
        if(node.GetType() == EGraphPassType::Blit) {
            const BlitNodeContext& currentNodeContext = this->GetContext<BlitNodeContext>();
            
            // Texture Resources
            readTextureResources.insert(readTextureResources.end(), currentNodeContext._readResources._textureResources.begin(), currentNodeContext._readResources._textureResources.end());
            
            // Buffer Resources
            readBufferResources.insert(readBufferResources.end(), currentNodeContext._readResources._buffersResources.begin(), currentNodeContext._readResources._buffersResources.begin());
        }

        if(node.GetType() == EGraphPassType::Raster) {
            const RasterNodeContext& incomingNodeContext = node.GetContext<RasterNodeContext>();
            
            // Texture Resources
            writeTextureResources.insert(writeTextureResources.end(), incomingNodeContext._writeResources._textureResources.begin(), incomingNodeContext._writeResources._textureResources.end());
            
            // Buffer Resources
            writeBufferResources.insert(writeBufferResources.end(), incomingNodeContext._writeResources._buffersResources.begin(), incomingNodeContext._writeResources._buffersResources.end());
        }
        
        bool bFoundDependency = false;
        for (auto readTextureResource : readTextureResources) {
            bFoundDependency |= std::find(writeTextureResources.begin(), writeTextureResources.end(), readTextureResource) != writeTextureResources.end();
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
    void AddRasterPass(const std::string& passName, const GraphicsPipelineParams& pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback&& callback);
    
    template <typename T> requires(!AcceptRasterPassIf<T>)
    void AddRasterPass(const std::string&, const GraphicsPipelineParams&, const RenderAttachments&, const CommandCallback&&) {
        static_assert(AcceptRasterPassIf<T>, "AddRaster pass function being called with wrong material component template parameter");
    };

    void AddBlitPass(std::string passName, const BlitCommandCallback &&callback) const;
            
    void Exectue(std::function<void(RenderGraphNode)> func);
        
protected:
    std::vector<RenderGraphNode> _nodes;

private:
    DirectGraph<RenderGraphNode> _graph;
    GraphicsContext* _graphicsContext;
    DAG _dag;
};
