#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/RenderPass/RenderPassInterface.hpp"

GraphBuilder::GraphBuilder(GraphicsContext* graphicsContext)
    : _graphicsContext(graphicsContext) {
}

void GraphBuilder::AddRasterPass(Scene *scene, RenderPass *renderPass, const RasterRenderFunction &callback) {
    renderPass->Initialize(_graphicsContext);

    // TODO this load textures is happening too soon? Right now we need to assume that we want to load every texture
    // but the correct thing is to only load if the meshes are going to be used.. not everything needs to be drawn if its not in view
    // I guess we still need a prestep to decide what resources to load based on visibility, this GetTextureResources could only
    // return resources that are in the frustrum and are going to be drawn.
    
    // New idea, we need a custom compute pass that will identify what entities will be visible, we then load the textures based on that
    // so this code should be move to that logic and not here, for now it works
    auto textures = renderPass->GetTextureResources(scene);
    
    PassResources passResourceReads;
    passResourceReads._textures.resize(textures.size());
    std::copy(textures.begin(), textures.end(), passResourceReads._textures.begin());
    
    // Load from disk all textures necessary for this pass. TODO: Make it parallel
    for (std::shared_ptr<Texture2D> texture : passResourceReads._textures) {
        texture->Initialize(_graphicsContext->GetDevice());
        texture->Reload();
    }
    
    MakeImplicitBlitTransfer(passResourceReads);
    
    PassResources passResourceWrites;
        
    if(renderPass->GetRenderAttachments(_graphicsContext)._colorAttachmentBinding.has_value())
        passResourceWrites._textures.push_back(renderPass->GetRenderAttachments(_graphicsContext)._colorAttachmentBinding->_texture);
    
    if(renderPass->GetRenderAttachments(_graphicsContext)._depthStencilAttachmentBinding.has_value())
        passResourceWrites._textures.push_back(renderPass->GetRenderAttachments(_graphicsContext)._depthStencilAttachmentBinding->_texture);
    
    RasterNodeContext context;
    context._renderAttachments = renderPass->GetRenderAttachments(_graphicsContext);
    context._pipeline = renderPass->GetGraphicsPipeline();
    context._callback = callback;
    context._passName = renderPass->GetIdentifier();
    context._readResources = std::move(passResourceReads);
    context._writeResources = std::move(passResourceWrites);
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

template <typename T> requires(AcceptRasterPassIf<T>)
void GraphBuilder::AddRasterPass(const std::string& passName, Scene* scene, const GraphicsPipelineParams &pipelineParams, const RenderAttachments& renderAttachments, const RasterRenderFunction &callback) {
        
    // Generates the shaders if necessary
    MaterialProcessor<T>::GenerateShaders(_graphicsContext);
    
    GraphicsPipelineParams params = pipelineParams;
//    params._graphicsContext = _graphicsContext;
    params._renderAttachments = renderAttachments;
//    params._shaderParams._vertexShader = MaterialProcessor<T>::GetVertexShader(_graphicsContext);
//    params._shaderParams._fragmentShader = MaterialProcessor<T>::GetFragmentShader(_graphicsContext);

    auto pipeline = GraphicsPipeline::Create(params);
    pipeline->Compile();
            
    // Not sure if i should use material textures as the render graph dependency tree
    // ? Instead this should be render targets produced by previous passes ?
    auto textures = MaterialProcessor<T>::GetTextures(_graphicsContext, scene);

    PassResources passResourceReads;
    passResourceReads._textures.resize(textures.size());
    std::copy(textures.begin(), textures.end(), passResourceReads._textures.begin());

    // Load from disk all textures necessary for this pass. TODO: Make it parallel
    for (std::shared_ptr<Texture2D> texture : passResourceReads._textures) {
        texture->Initialize(_graphicsContext->GetDevice());
        texture->Reload();
    }
    
    MakeImplicitBlitTransfer(passResourceReads);
    
    PassResources passResourceWrites;
    
    if(renderAttachments._colorAttachmentBinding.has_value())
        passResourceWrites._textures.push_back(renderAttachments._colorAttachmentBinding->_texture);
    
    if(renderAttachments._depthStencilAttachmentBinding.has_value())
        passResourceWrites._textures.push_back(renderAttachments._depthStencilAttachmentBinding->_texture);
    
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

void GraphBuilder::AddBlitPass(std::string passName, PassResources resources, const BlitCommandCallback &callback) {
    BlitNodeContext context;
    context._passName = passName;
    context._callback = callback;
    context._writeResources = resources;
    
    RenderGraphNode node;
    node._ctx = context;
    
    _nodes.push_back(node);
}

void GraphBuilder::AddBlitPass(std::string passName, PassResources readResources, PassResources writeResources, const BlitCommandCallback &callback) {
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

void GraphBuilder::MakeImplicitBlitTransfer(const PassResources& passResources) {
    AddBlitPass("ImplicitResourcesTransfer", passResources, [passResources](BlitCommandEncoder* enc, PassResources readResources, PassResources writeResources) {
        for(const auto texture : passResources._textures) {
            if(texture && texture->IsDirty()) {
                enc->MakeImageBarrier(texture.get(), ImageLayout::LAYOUT_TRANSFER_DST);
                enc->UploadImageBuffer(texture);
                enc->MakeImageBarrier(texture.get(), ImageLayout::LAYOUT_SHADER_READ);
            }
        }
        
        // TODO: Make the same for buffers
    });
}

// Explicit instantiation for specific types
template void GraphBuilder::AddRasterPass<MatCapMaterialComponent>(const std::string&, Scene*, const GraphicsPipelineParams&, const RenderAttachments&, const RasterRenderFunction&);
template void GraphBuilder::AddRasterPass<PhongMaterialComponent>(const std::string&, Scene*, const GraphicsPipelineParams&, const RenderAttachments&, const RasterRenderFunction&);
template void GraphBuilder::AddRasterPass<GridMaterialComponent>(const std::string&, Scene*, const GraphicsPipelineParams&, const RenderAttachments&, const RasterRenderFunction&);

