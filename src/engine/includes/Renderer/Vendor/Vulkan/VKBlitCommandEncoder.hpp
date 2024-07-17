#pragma once
#include "Renderer/CommandEncoders/BlitCommandEncoder.hpp"

class VKBlitCommandEncoder : public BlitCommandEncoder {
public:
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
};
