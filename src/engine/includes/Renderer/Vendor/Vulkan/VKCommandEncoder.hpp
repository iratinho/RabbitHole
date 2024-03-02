#pragma once
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKCommandEncoder : public CommandEncoder {
public:    
    void SetViewport() override {};
};
