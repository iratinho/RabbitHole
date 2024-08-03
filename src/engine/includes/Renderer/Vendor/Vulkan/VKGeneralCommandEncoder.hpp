#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKGeneralCommandEncoder : public virtual GeneralCommandEncoder {
public:
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
};
