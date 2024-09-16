#pragma once
#include "Renderer/GPUDefinitions.h"

class CommandBuffer;
class Device;
class GraphicsContext;
class Shader;

class GeneralCommandEncoder {
public:
    GeneralCommandEncoder() {};
    
    GeneralCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device)
        : _graphicsContext(graphicsContext)
        , _device(device)
        , _commandBuffer(commandBuffer)
    {}
    
    virtual ~GeneralCommandEncoder();
    virtual void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) = 0;
    virtual void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) = 0;
    virtual GraphicsContext* GetGraphisContext() { return _graphicsContext; }
protected:
    GraphicsContext* _graphicsContext;
    Device* _device;
    CommandBuffer* _commandBuffer;
    
    friend class CommandBuffer;
};
