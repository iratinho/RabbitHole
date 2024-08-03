#pragma once
#include "Renderer/GPUDefinitions.h"

class CommandBuffer;
class RenderContext;

class GeneralCommandEncoder {
public:
    virtual ~GeneralCommandEncoder() = default;
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
    
protected:
    std::shared_ptr<RenderContext> _renderContext;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;
};
