#pragma once
#include "Renderer/CommandBuffer.hpp"

class VKCommandBuffer : public CommandBuffer {
public:
    VKCommandBuffer(CommandQueue* commandQueue);

    void Submit() override;
};
