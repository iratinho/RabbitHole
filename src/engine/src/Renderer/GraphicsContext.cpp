#include "Renderer/GraphicsContext.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/RenderPass.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Texture2D.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#endif

// TODO create the ideia of graphics, copy and compute queues

GraphicsContext::GraphicsContext() {
}

GraphicsContext::~GraphicsContext() {}

std::unique_ptr<GraphicsContext> GraphicsContext::Create(Device* device) {
#ifdef USING_VULKAN_API
    auto instance = std::make_unique<VKGraphicsContext>(device);
//    instance->_commandEncoder = CommandEncoder::MakeCommandEncoder(renderContext);
    
    return instance;
#endif
    
    return nullptr;
}
