#include "Renderer/GraphicsContext.hpp"
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#endif

// TODO create the ideia of graphics, copy and compute queues

GraphicsContext::GraphicsContext()
    : _graphBuilder(this) {
}

GraphicsContext::~GraphicsContext() {}

std::shared_ptr<GraphicsContext> GraphicsContext::Create(std::shared_ptr<RenderContext> renderContext) {
#ifdef USING_VULKAN_API
    auto instance = std::make_shared<VKGraphicsContext>(renderContext);
    instance->_commandEncoder = CommandEncoder::MakeCommandEncoder(renderContext);
    
    return instance;
#endif
    
    return nullptr;
}

void GraphicsContext::Flush() {
    _graphBuilder.Exectue([this](RenderGraphNode node) {
        Execute(node);
    });
}
