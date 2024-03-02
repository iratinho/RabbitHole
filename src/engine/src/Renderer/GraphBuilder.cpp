#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"

GraphBuilder::GraphBuilder(GraphicsContext* graphicsContext)
    : _graphicsContext(graphicsContext) {
}

void GraphBuilder::AddRasterPass(const GraphicsPipelineParams &pipelineParams, const RasterPassTarget &colorTarget, const RasterPassTarget &depthTarget, const CommandCallback &&callback) {
    
    GraphicsPipelineParams params = pipelineParams;
    params._graphicsContext = _graphicsContext;
    params._colorAttachment = colorTarget._attachmentInfo;
    params._depthAttachment = depthTarget._attachmentInfo;
    
    auto pipeline = GraphicsPipeline::Create(params);
    pipeline->Compile();

    RenderPassContext context;
    context._colorTarget = colorTarget;
    context._depthTarget = depthTarget;
    context._pipeline = pipeline.get();
    context._callback = callback;
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::Exectue(GraphicsContext* context) {
    // Build the graph
    for(size_t i = 0; i < _nodes.size(); i++) {
        _dag.MakeVertex(i);
        size_t oHash = _nodes[i].GetOutputHash();
        for(size_t x = 0; x < _nodes.size(); x++) {
            size_t iHash = _nodes[x].GetInputHash();
            if(oHash == iHash) {
                _dag.MakeEdge({i, x});
            }
        }
    }
    
    _dag.Sort();
    _dag.ForEachSorted([this, context](DAG::Vertex v) {
        context->Execute(_nodes[v]);
    });
}
