#pragma once
#include "Renderer/CommandEncoders/GeneralCommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class Shader;
class GraphicsContext;
class Device;
class VKGraphicsContext;

class VKGeneralCommandEncoder : public GeneralCommandEncoder {
public:
    VKGeneralCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device)
        : GeneralCommandEncoder(commandBuffer, graphicsContext, device)
    {}
    
    void MakeImageBarrier(Texture2D* texture2D, ImageLayout after) override;
    void BindShaderResources(Shader* shader, const ShaderInputResourceUSet& resources) override;
    
private:
    VKGraphicsContext* GetVKContext();
};
