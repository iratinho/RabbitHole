#include "Renderer/CommandBuffer.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Event.hpp"
#include "Renderer/Fence.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#else
#include "Renderer/Vendor/WebGPU/WebGPUCommandBuffer.hpp"
#endif

CommandBuffer::~CommandBuffer() = default;

std::unique_ptr<CommandBuffer> CommandBuffer::MakeCommandBuffer(const CommandBuffer::InitializationParams& params) {
    std::unique_ptr<CommandBuffer> instance;
    
#ifdef VULKAN_BACKEND
    instance = std::make_unique<VKCommandBuffer>();
    instance->_params = params;
#else
    instance = std::make_unique<WebGPUCommandBuffer>();
    instance->_params = params;
#endif
    
    if(instance) {
        instance->Initialize();
    }
    
    return instance;
}

RenderCommandEncoder* CommandBuffer::MakeRenderCommandEncoder(GraphicsContext* graphicsContext, Device* device) {
    auto encoder = RenderCommandEncoder::MakeCommandEncoder(this, graphicsContext, device);
    return _renderCommandEncoders.emplace_back(std::move(encoder)).get();
}

BlitCommandEncoder* CommandBuffer::MakeBlitCommandEncoder(GraphicsContext* graphicsContext, Device* device) {
    auto encoder = BlitCommandEncoder::MakeCommandEncoder(this, graphicsContext, device);
    return _blitCommandEncoders.emplace_back(std::move(encoder)).get();
}

void CommandBuffer::EncodeWaitForEvent(std::shared_ptr<Event> event) {
    _waitEvents.push_back(event);
}

void CommandBuffer::EncodeSignalEvent(std::shared_ptr<Event> event) {
    _signalEvents.push_back(event);
}

void CommandBuffer::Submit(std::shared_ptr<Fence> fence) {
//    _signalEvents.clear();
//    _waitEvents.clear();
}

void CommandBuffer::RemoveEncoder(RenderCommandEncoder *ptr) {
    _renderCommandEncoders.clear();
}

void CommandBuffer::RemoveEncoder(BlitCommandEncoder *ptr) {
    _blitCommandEncoders.clear();
}

