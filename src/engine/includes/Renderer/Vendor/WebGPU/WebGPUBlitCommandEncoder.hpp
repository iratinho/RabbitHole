#pragma once
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "webgpu/webgpu.hpp"

class WebGPUBlitCommandEncoder : public BlitCommandEncoder {
public:
    WebGPUBlitCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device);
    ~WebGPUBlitCommandEncoder();
    
    void BeginBlitPass() override;
    void EndBlitPass() override;
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
    void UploadImageBuffer(std::shared_ptr<Texture2D> texture) override;
    void CopyImageToImage(const std::shared_ptr<Texture2D>& src, const std::shared_ptr<Texture2D>& dst) override;

    [[nodiscard]] WGPUCommandEncoder GetWebGPUEncoder() const {
        return _encoder;
    };

private:
    WGPUCommandEncoder _encoder = nullptr;
};
