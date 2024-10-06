#include "Renderer/RenderSystemV2.hpp"
#include "Renderer/RenderPass/RenderPassRegistration.inl"
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/Processors/TransformProcessor.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Core/Scene.hpp"
#include "Core/Utils.hpp"
#include "window.hpp"

RenderSystemV2::RenderSystemV2() {
}

RenderSystemV2::~RenderSystemV2() {
}

bool RenderSystemV2::Initialize(Window* window) {
    _windowsContexts[window] = 0;
    
    return true;
}

// TODO: We need a AddWindow function to add windows to the map
bool RenderSystemV2::Process(Scene* scene) {
    for(auto& [window, currentContext] : _windowsContexts) {
        if(currentContext > 1) {
            currentContext = 0;
        }
    
        auto ctx = window->GetDevice()->GetGraphicsContext(currentContext);
        
        BeginFrame(ctx, scene);
        Render(ctx, scene);
        EndFrame(ctx);
        
        currentContext++;
    }
    
    return true;
}

void RenderSystemV2::RegisterRenderPass(RenderPass *pass) {
    GetRenderPasses().push_back(pass);
}

void RenderSystemV2::BeginFrame(GraphicsContext* graphicsContext, Scene* scene) {
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    // Create a new graph builder per frame, this as no cost
    _graphBuilder = GraphBuilder(graphicsContext);
    
    // Upload to GPU side all geometry resources
    if(auto buffer = MeshProcessor::GenerateBuffer(graphicsContext->GetDevice(), scene)) {
        auto blitCallback = [buffer](Encoders encoders, const PassResources& read, const PassResources& write) {
            encoders._blitEncoder->UploadBuffer(buffer);
        };
        
        PassResources resources;
        resources._buffersResources.push_back(buffer);
        
        _graphBuilder.AddBlitPass("Upload geometry buffers", resources, blitCallback);
    }

    // Updates all transforms in the scene to be used when rendering
    TransformProcessor::Process(scene);
    
    graphicsContext->BeginFrame();
}

void RenderSystemV2::Render(GraphicsContext* graphicsContext, Scene* scene) {
    if(!graphicsContext) {
        return;
    }
    
    // Process render passes
    for(RenderPass* pass : GetRenderPasses()) {
        // TODO: Pass this logic to inside the AddRasterPass, this way we only need to send the pass
        GraphicsPipelineParams pipelineParams = pass->GetPipelineParams();
        RenderAttachments renderAttachments = pass->GetRenderAttachments(graphicsContext);

        auto RenderFunc = [this, graphicsContext, scene, pass](Encoders encoders, class GraphicsPipeline* pipeline) {
            pass->Process(encoders, scene, pipeline);
        };
        
        _graphBuilder.AddRasterPass(scene, pass, RenderFunc);
    }
    
    auto ExecutePass = [this, graphicsContext](RenderGraphNode node){
        if(node.GetType() == EGraphPassType::Raster || node.GetType() == EGraphPassType::Blit) {
            graphicsContext->Execute(node);
        }
    };
    
    _graphBuilder.Exectue(ExecutePass);
}

void RenderSystemV2::EndFrame(GraphicsContext* graphicsContext) {
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    graphicsContext->EndFrame();
    graphicsContext->Present();
}
