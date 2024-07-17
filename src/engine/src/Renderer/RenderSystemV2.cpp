
#include "Components/DirectionalLightComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/MatCapMaterialComponent.hpp"

#include "Renderer/RenderPass/RenderPassRegistration.inl"
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/Processors/TransformProcessor.hpp"
#include "Renderer/Processors/GeometryProcessors.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/RenderSystemV2.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/Swapchain.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Buffer.hpp"

#include "Core/MeshObject.hpp"
#include "Core/Camera.hpp"
#include "Core/Scene.hpp"
#include "Core/Utils.hpp"

RenderSystemV2::RenderSystemV2() {
}

RenderSystemV2::~RenderSystemV2() {
}

bool RenderSystemV2::Initialize(const InitializationParams& params) {
    _device = std::make_shared<Device>(nullptr);
    if(!_device->Initialize(params)) {
        return false;
    }

    // Create a new graphics context per swapchain image
    constexpr unsigned int swapChainCount = 2;
    for (size_t i = 0; i < swapChainCount; i++) {
        auto context = GraphicsContext::Create(_device);
        if(!context->Initialize()) {
            return false;
        }
        
        _graphicsContext.push_back(context);
    }
            
    return true;
}

bool RenderSystemV2::Process(Scene* scene) {
    if(currentContext > 1) {
        currentContext = 0;
    }
    
    BeginFrame(scene);
    Render(scene);
    EndFrame();
    
    currentContext++;
    
    return true;
}

void RenderSystemV2::RegisterRenderPass(IRenderPass *pass) {
    GetRenderPasses().push_back(pass);
}

void RenderSystemV2::BeginFrame(Scene* scene) {
    auto graphicsContext = _graphicsContext[currentContext];
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    // Create a new graph builder per frame, this as no cost
    _graphBuilder = GraphBuilder(graphicsContext.get());
    
    // Upload to GPU side all geometry resources
    auto buffer = MeshProcessor::GenerateBuffer(_device.get(), scene);
    if(buffer) {
        auto blitCallback = [buffer](BlitCommandEncoder* encoder, const PassResources& read, const PassResources& write) {
            encoder->UploadBuffer(buffer);
        };
        
        PassResources resources;
        resources._buffersResources.push_back(buffer);
        
        _graphBuilder.AddBlitPass("Upload geometry buffers", resources, blitCallback);
    }

    // Updates all transforms in the scene to be used when rendering
    TransformProcessor::Process(scene);
    
    graphicsContext->BeginFrame();
}

void RenderSystemV2::Render(Scene* scene) {
    auto graphicsContext = _graphicsContext[currentContext].get();
    if(!graphicsContext) {
        return false;
    }
    
    for(auto renderPass : GetRenderPasses()) {
        renderPass->Setup(&_graphBuilder, graphicsContext, scene);
    }
            
    // TODO: Can we improve this by creating a flush function inside the graph builder? This would make the render system cleaner
    // Execute all rendering commands
    _graphBuilder.Exectue([this, graphicsContext](RenderGraphNode node) {
        // TODO: Right now we only support working on the same command buffer, but i want to have other command buffers
        // TODO: so we need a way to sync them for individual execution
        bool bSupportsTransferQueue = false; // Check from device caps
        if(node.GetType() == EGraphPassType::Raster || (!bSupportsTransferQueue && node.GetType() == EGraphPassType::Blit)) {
            graphicsContext->Execute(node);
        }
        
        if(bSupportsTransferQueue && node.GetType() == EGraphPassType::Blit) {
            // TODO: New context specialized just for blit operations, the device needs to have support for it
        }
    });
}

void RenderSystemV2::EndFrame() {
    auto& graphicsContext = _graphicsContext[currentContext];
    if(!graphicsContext) {
        assert(0);
        return;
    }
    
    graphicsContext->EndFrame();
    graphicsContext->Present();
}
