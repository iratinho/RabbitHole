#pragma once
#include "Renderer/BlitCommandEncoder.hpp"

class VKBlitCommandEncoder : public BlitCommandEncoder {
public:
    void UploadBuffer(std::shared_ptr<Buffer> buffer) override;
};
