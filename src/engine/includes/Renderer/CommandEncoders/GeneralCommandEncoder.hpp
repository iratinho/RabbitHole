#pragma once
#include "Renderer/GPUDefinitions.h"

class CommandBuffer;
class RenderContext;
class GraphicsContext;
class Shader;

class GeneralCommandEncoder {
public:
    GeneralCommandEncoder() {};
    
    GeneralCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext)
        : _graphicsContext(graphicsContext)
        , _renderContext(renderContext)
        , _commandBuffer(commandBuffer)
    {}
    
    virtual ~GeneralCommandEncoder();
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
    virtual void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) = 0;
    virtual GraphicsContext* GetGraphisContext() { return _graphicsContext; }
protected:
    GraphicsContext* _graphicsContext;
    RenderContext* _renderContext;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;
};
