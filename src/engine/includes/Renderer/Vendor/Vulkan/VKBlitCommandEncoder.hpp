#pragma once
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"
#include "Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"

class Texture2D;

class VKBlitCommandEncoder : public BlitCommandEncoder, public VKGeneralCommandEncoder {
public:
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
    void UploadImageBuffer(std::shared_ptr<Texture2D> texture) override;
};
