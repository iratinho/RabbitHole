#pragma once
#include "Core/Utils.hpp"
#include "CommandEncoder.hpp"
#include "GPUDefinitions.h"

struct GraphicsPipelineParams;
class GraphicsPipeline;
class GraphicsContext;

class RenderGraphNode {
public:
    std::size_t GetHash() const {
        return 0;
    }
    
    std::size_t GetInputHash() const {
        std::hash<std::string> hasher;
        return hasher(_ctx._renderAttachments._identifier);
    }
    
    std::size_t GetOutputHash() const {
        return 0;
    }
    
    RenderPassContext* GetContext() {
        return &_ctx;
    }
    
private:
    RenderPassContext _ctx;
    friend class GraphBuilder;
};

class GraphBuilder {
public:
    GraphBuilder(GraphicsContext* graphicsContext);
    
    void AddRasterPass(const GraphicsPipelineParams& pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback&& callback);
    
    void Exectue(GraphicsContext* context);
        
private:
    DirectGraph<RenderGraphNode> _graph;
    GraphicsContext* _graphicsContext;
    std::vector<RenderGraphNode> _nodes;
    DAG _dag;
    
    // TODO we should store the nodes here
    // the graph should connect vertices based on this nodes indices, this will make the graph agnostic to
    // rendering
};
