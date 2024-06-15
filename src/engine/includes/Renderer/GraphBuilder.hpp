#pragma once
#include "Core/Utils.hpp"
#include "GPUDefinitions.h"
#include "GraphicsPipeline.hpp"

struct GraphicsPipelineParams;
class TextureResource;

enum class EGraphPassType {
    None,
    Raster,
    Transfer
};

struct PassResources {
    std::vector<std::shared_ptr<TextureResource>> _textureResources;
    std::shared_ptr<Buffer> _buffers;
};

struct RasterNodeContext {
public:
    std::size_t GetInputHash() const {
        std::hash<std::string> hasher;
        return hasher(_renderAttachments._identifier);
    }

public:
    RasterPassTarget _input;
    RenderAttachments _renderAttachments;
    
    PassResources _readResources;
    PassResources _writeResources;
    
    CommandCallback _callback;
    class GraphicsPipeline* _pipeline;
    std::string _passName;
};

struct TransferNodeContext {
public:
    std::size_t GetInputHash() const {
        return {};
    }
    };

class RenderGraphNode {
public:
    std::size_t GetHash() const {
        return 0;
    }
    
    std::size_t GetOutputHash() const {
        return 0;
    }
    
    EGraphPassType GetType() {
        if(std::holds_alternative<RasterNodeContext>(_ctx)) {
            return EGraphPassType::Raster;
        }
        
        if(std::holds_alternative<TransferNodeContext>(_ctx)) {
            return EGraphPassType::Transfer;
        }

        assert(0);
        return EGraphPassType::None;
    };
    
    template <typename ContextType>
    ContextType& GetContext() { return std::get<ContextType>(_ctx); };
        
    const std::size_t GetInputHash() {
        switch (GetType()) {
            case EGraphPassType::Raster:
                return GetContext<RasterNodeContext>().GetInputHash();
                break;
            case EGraphPassType::Transfer:
                return GetContext<TransferNodeContext>().GetInputHash();
                break;
                
            default:
                assert(0);
                break;
        }

        return 0;
    }
    
    bool DependesOn(RenderGraphNode& node) {
        // Need to revisit this...
        if(this->GetType() == EGraphPassType::Raster && node.GetType() == EGraphPassType::Raster) {
            const RasterNodeContext& currentNodeContext = this->GetContext<RasterNodeContext>();
            const RasterNodeContext& incomingNodeContext = node.GetContext<RasterNodeContext>();
            
            std::vector<std::shared_ptr<TextureResource>> readTextureResources = currentNodeContext._readResources._textureResources;
            std::vector<std::shared_ptr<TextureResource>> writeTextureResources = incomingNodeContext._writeResources._textureResources;
            
            bool bFoundDependency = false;
            for (auto readTextureResource : readTextureResources) {
                bFoundDependency |= std::find(writeTextureResources.begin(), writeTextureResources.end(), readTextureResource) != writeTextureResources.end();
            }
            
            return bFoundDependency;
            
            
            
            // If both passes uses the same attachments there is a direct relationship between them, in this case we use the order of insertion
            // its the one that was created first, unless there are other resources in play
//            if((this->GetContext<RasterNodeContext>()._renderAttachments._colorAttachmentBinding->_texture == node.GetContext<RasterNodeContext>()._renderAttachments._colorAttachmentBinding->_texture) &&
//               (this->GetContext<RasterNodeContext>()._renderAttachments._depthStencilAttachmentBinding->_texture == node.GetContext<RasterNodeContext>()._renderAttachments._depthStencilAttachmentBinding->_texture)) {
//                return true;
//            }
//            if(this->GetContext<RasterNodeContext>()._renderAttachments._depthStencilAttachmentBinding->_depthLoadAction == LoadOp::OP_LOAD
//               && node.GetContext<RasterNodeContext>()._renderAttachments._depthStencilAttachmentBinding->_depthLoadAction == LoadOp::OP_CLEAR) {
//                return true;
//            }
        }
        
        return false;
    }

protected:
    EGraphPassType _graphPassType;
    std::variant<RasterNodeContext, TransferNodeContext> _ctx;
    
    friend class GraphBuilder;
};

class GraphBuilder {
public:
    GraphBuilder() {};
    GraphBuilder(GraphicsContext* graphicsContext);
    
    template <typename MaterialComponent>
    void AddRasterPass(std::string passName, const GraphicsPipelineParams& pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback&& callback);
        
    void Exectue(std::function<void(RenderGraphNode)> func);
        
protected:
    std::vector<RenderGraphNode> _nodes;

private:
    DirectGraph<RenderGraphNode> _graph;
    GraphicsContext* _graphicsContext;
    DAG _dag;
};
