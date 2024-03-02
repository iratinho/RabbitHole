#pragma once
#include "Renderer/CommandQueue.hpp"

class VKCommandQueue : public CommandQueue {
public:
    CommandBuffer* MakeCommandBuffer() override;
};
