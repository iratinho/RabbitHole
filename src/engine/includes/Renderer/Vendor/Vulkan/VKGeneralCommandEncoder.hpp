#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class Shader;
class GraphicsContext;
class RenderContext;
class VKGraphicsContext;

class VKGeneralCommandEncoder : public GeneralCommandEncoder {
public:
    VKGeneralCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, RenderContext* renderContext)
        : GeneralCommandEncoder(commandBuffer, graphicsContext, renderContext)
    {}
    
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
    void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) override;
    
private:
    VKGraphicsContext* GetVKContext();
};
