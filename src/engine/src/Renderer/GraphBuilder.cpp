#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"

GraphBuilder::GraphBuilder(GraphicsContext* graphicsContext)
    : _graphicsContext(graphicsContext) {
}

template <typename T> requires(AcceptRasterPassIf<T>)
void GraphBuilder::AddRasterPass(const std::string& passName, const GraphicsPipelineParams &pipelineParams, const RenderAttachments& renderAttachments, const CommandCallback &&callback) {
        
    // Generates the shaders if necessary
    MaterialProcessor<T>::GenerateShaders(_graphicsContext);
    
    GraphicsPipelineParams params = pipelineParams;
    params._graphicsContext = _graphicsContext;
    params._renderAttachments = renderAttachments;
    params._vertexShader = MaterialProcessor<T>::GetVertexShader(_graphicsContext);
    params._fragmentShader = MaterialProcessor<T>::GetFragmentShader(_graphicsContext);

    auto pipeline = GraphicsPipeline::Create(params);
    pipeline->Compile();
        
    //TODO Load textures and add a new implicit blit pass to upload the textures to the GPU
    
    PassResources passResourceReads;
    PassResources passResourceWrites;
    
    passResourceReads._textureResources = MaterialProcessor<T>::GetTextureResources(_graphicsContext);
        
    if(renderAttachments._colorAttachmentBinding.has_value())
        passResourceWrites._textureResources.push_back(renderAttachments._colorAttachmentBinding->_texture->GetResource());
    
    if(renderAttachments._depthStencilAttachmentBinding.has_value())
        passResourceWrites._textureResources.push_back(renderAttachments._depthStencilAttachmentBinding->_texture->GetResource());
    
    RasterNodeContext context;
    context._renderAttachments = renderAttachments;
    context._pipeline = pipeline.get();
    context._callback = callback;
    context._passName = passName;
    context._readResources = std::move(passResourceReads);
    context._writeResources = std::move(passResourceWrites);
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::AddBlitPass(std::string passName, PassResources resources, const BlitCommandCallback &&callback) {
    BlitNodeContext context;
    context._passName = passName;
    context._callback = callback;
    context._writeResources = resources;
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::AddBlitPass(std::string passName, PassResources readResources, PassResources writeResources, const BlitCommandCallback &&callback) {
    BlitNodeContext context;
    context._passName = passName;
    context._callback = callback;
    context._readResources = readResources;
    context._writeResources = writeResources;
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::Exectue(std::function<void(RenderGraphNode)> func) {
    // Build the graph
    for(size_t o = 0; o < _nodes.size(); o++) {
        bool bWasInserted = false;
        size_t outputIndex = o + 1;
        for(size_t i = 0; i < _nodes.size(); i++) {
            size_t inputIndex = i + 1;
            if(outputIndex == inputIndex) continue;
            if(_nodes[inputIndex - 1].DependesOn(_nodes[outputIndex - 1])) {
                _dag.MakeEdge({outputIndex, inputIndex});
                bWasInserted = true;
            }
        }
            
        if(!bWasInserted) {
            _dag.MakeEdge({0, outputIndex});
        }
    }
    
    _dag.Sort();
    _dag.ForEachSorted([this, func](DAG::Vertex v) {
        func(_nodes[v - 1]);
    });
    
    _nodes.clear();
    _dag.Clear();
}

// Explicit instantiation for specific types
template void GraphBuilder::AddRasterPass<MatCapMaterialComponent>(const std::string&, const GraphicsPipelineParams&, const RenderAttachments&, const CommandCallback&&);

template void GraphBuilder::AddRasterPass<PhongMaterialComponent>(const std::string&, const GraphicsPipelineParams&, const RenderAttachments&, const CommandCallback&&);

template void GraphBuilder::AddRasterPass<GridMaterialComponent>(const std::string&, const GraphicsPipelineParams&, const RenderAttachments&, const CommandCallback&&);
