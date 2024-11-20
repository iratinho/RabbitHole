#include "Renderer/GraphicsContext.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUGraphicsContext.hpp"
#endif

// TODO create the ideia of graphics, copy and compute queues

GraphicsContext::GraphicsContext() {
}

GraphicsContext::~GraphicsContext() {}

std::unique_ptr<GraphicsContext> GraphicsContext::Create(Device* device) {
#ifdef VULKAN_BACKEND
    auto instance = std::make_unique<VKGraphicsContext>(device);
//    instance->_commandEncoder = CommandEncoder::MakeCommandEncoder(renderContext);
    
    return instance;
#else
    auto instance = std::make_unique<WebGPUGraphicsContext>(device);
    return instance;
#endif
    
    return nullptr;
}
