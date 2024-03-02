#pragma once
#include "Renderer/GPUDefinitions.h"

class RenderContext;

class CommandEncoder {
public:
    virtual ~CommandEncoder() = default;
    
    static std::unique_ptr<CommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);
    
    virtual void SetViewport() = 0;
    
protected:
    std::shared_ptr<RenderContext> _renderContext;
};
