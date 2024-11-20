#include "Renderer/GraphBuilder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"

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
    std::ranges::copy(textures, passResourceReads._textures.begin());
    
    // Load from disk all textures necessary for this pass. TODO: Make it parallel
    for (const std::shared_ptr<Texture2D>& texture : passResourceReads._textures) {
        if(texture) {
            texture->Initialize(_graphicsContext->GetDevice());
            texture->Reload();
        }
    }
    
    MakeImplicitBlitTransfer(passResourceReads);
    
    PassResources passResourceWrites;
        
    passResourceWrites._textures.push_back(renderPass->GetRenderAttachments(_graphicsContext)._colorAttachmentBinding._texture);
    
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
    AddBlitPass("ImplicitResourcesTransfer", passResources, [passResources](Encoders encoders, PassResources readResources, PassResources writeResources) {
        for(const auto texture : passResources._textures) {
            if(texture && texture->IsDirty()) {
                encoders._renderEncoder->MakeImageBarrier(texture.get(), ImageLayout::LAYOUT_TRANSFER_DST);
                encoders._renderEncoder->UploadImageBuffer(texture);
                encoders._renderEncoder->MakeImageBarrier(texture.get(), ImageLayout::LAYOUT_SHADER_READ);
            }
        }
        
        // TODO: Make the same for buffers
    });
}
