#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/RenderTarget.hpp"

GraphBuilder::GraphBuilder(GraphicsContext* graphicsContext)
    : _graphicsContext(graphicsContext) {
}

void GraphBuilder::AddRasterPass(std::string passName, const GraphicsPipelineParams &pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback &&callback) {
    GraphicsPipelineParams params = pipelineParams;
    params._graphicsContext = _graphicsContext;
    params._renderAttachments = renderAttachments;
    
    auto pipeline = GraphicsPipeline::Create(params);
    pipeline->Compile();

    RenderPassContext context;
    context._renderAttachments = renderAttachments;
    context._pipeline = pipeline.get();
    context._callback = callback;
    context._passName = passName;
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::Exectue(GraphicsContext* context) {
    // Build the graph
    for(size_t o = 0; o < _nodes.size(); o++) {
        _dag.MakeVertex(o);
        size_t oHash = _nodes[o].GetOutputHash();
        LoadOp depthLoadActionO = _nodes[o].GetContext()->_renderAttachments._depthStencilAttachmentBinding->_depthLoadAction;
        for(size_t i = 0; i < _nodes.size(); i++) {
            if(o == i) continue;
            size_t iHash = _nodes[i].GetInputHash();
            LoadOp depthLoadActionI = _nodes[i].GetContext()->_renderAttachments._depthStencilAttachmentBinding->_depthLoadAction;
            // todo This loads ops here might not make much sense, it works for now, but we need to sort this better
            if(oHash == iHash || (depthLoadActionO == LoadOp::OP_CLEAR && depthLoadActionI == LoadOp::OP_LOAD)) {
                _dag.MakeEdge({o, i});
            }
        }
    }
    
    _dag.Sort();
    _dag.ForEachSorted([this, context](DAG::Vertex v) {
        context->Execute(_nodes[v]);
    });
    
    _nodes.clear();
    _dag.Clear();
}
