#include "Renderer/CommandBuffer.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Event.hpp"
#include "Renderer/Fence.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
#endif

CommandBuffer::~CommandBuffer() {};

std::unique_ptr<CommandBuffer> CommandBuffer::MakeCommandBuffer(const CommandBuffer::InitializationParams& params) {
    std::unique_ptr<CommandBuffer> instance;
    
#ifdef USING_VULKAN_API
    instance = std::make_unique<VKCommandBuffer>();
    instance->_params = params;
#endif
    
    if(instance) {
        instance->Initialize();
    }
    
    return instance;
}

RenderCommandEncoder* CommandBuffer::MakeRenderCommandEncoder(std::shared_ptr<RenderContext> renderContext) {
    auto encoder = RenderCommandEncoder::MakeCommandEncoder(renderContext);
    encoder->_commandBuffer = this;
    return _renderCommandEncoders.emplace_back(std::move(encoder)).get();
}

BlitCommandEncoder* CommandBuffer::MakeBlitCommandEncoder(std::shared_ptr<RenderContext> renderContext) {
    auto encoder = BlitCommandEncoder::MakeCommandEncoder(renderContext);
    encoder->_commandBuffer = this;
    return _blitCommandEncoders.emplace_back(std::move(encoder)).get();
}

void CommandBuffer::EncodeWaitForEvent(std::shared_ptr<Event> event) {
    _waitEvents.push_back(event);
}

void CommandBuffer::EncodeSignalEvent(std::shared_ptr<Event> event) {
    _signalEvents.push_back(event);
}

void CommandBuffer::Submit(std::shared_ptr<Fence> fence) {
    _signalEvents.clear();
    _waitEvents.clear();
}
