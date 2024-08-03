
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
    if(auto buffer = MeshProcessor::GenerateBuffer(_device.get(), scene)) {
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
    
    for(IRenderPass* pass : GetRenderPasses()) {
        pass->Setup(&_graphBuilder, graphicsContext, scene);
    }
    
    auto ExecutePass = [this, graphicsContext](RenderGraphNode node){
        if(node.GetType() == EGraphPassType::Raster || node.GetType() == EGraphPassType::Blit) {
            graphicsContext->Execute(node);
        }
    };
    
    _graphBuilder.Exectue(ExecutePass);
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
