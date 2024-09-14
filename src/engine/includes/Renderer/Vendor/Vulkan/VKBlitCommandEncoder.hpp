#pragma once
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"

class Texture2D;

class VKBlitCommandEncoder : public BlitCommandEncoder {
public:
    VKBlitCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext)
        : BlitCommandEncoder(commandBuffer, graphicsContext, renderContext)
    {}
    
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
    void UploadImageBuffer(std::shared_ptr<Texture2D> texture) override;
    
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
    void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) override;

};
