#pragma once
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKCommandEncoder : public CommandEncoder {
public:    
    void SetViewport(GraphicsContext *graphicsContext, int width, int height) override;
    void UpdatePushConstants(GraphicsContext *graphicsContext, GraphicsPipeline* graphicsPipeline, Shader *shader, const void *data) override;
    void DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy) override;
    void MakeImageBarrier(TextureResource* resource, ImageLayout before, ImageLayout after) override;

//    void ExecuteMemoryTransfer(Buffer* buffer) override;
};
