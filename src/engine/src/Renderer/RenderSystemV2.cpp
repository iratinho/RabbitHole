
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
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/TransferContext.hpp"
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

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_DIR) "/" STR(name)

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
    
    _transferContext = TransferContext::Create(_device.get());
        
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
        auto blitCallback = [](class CommandEncoder* encoder, const PassResources& read, const PassResources& write) {
            // TODO USE ENCODER TO DO THE TRANSFER WE WANT
        };
        
        PassResources resources;
        resources._buffersResources.push_back(buffer);
        
        _graphBuilder.AddBlitPass("Upload geometry buffers", resources, blitCallback);
    }

     if(buffer) {
         _transferContext->EnqueueBufferSync(buffer);
         _transferContext->Flush();
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
            
    // Execute all rendering commands
    _graphBuilder.Exectue([this, graphicsContext](RenderGraphNode node) {
        bool bSupportsTransferQueue = false; // Check from device caps
        if(node.GetType() == EGraphPassType::Raster || (!bSupportsTransferQueue && node.GetType() == EGraphPassType::Blit)) {
            graphicsContext->Execute(node);
        }
        
        if(bSupportsTransferQueue && node.GetType() == EGraphPassType::Blit) {
            // TODO New context specialized just for blit operations, the device needs to have support for it
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
