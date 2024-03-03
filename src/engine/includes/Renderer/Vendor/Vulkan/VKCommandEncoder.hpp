#pragma once
#include "Renderer/CommandEncoder.hpp"
#include "Renderer/GPUDefinitions.h"

class VKCommandEncoder : public CommandEncoder {
public:    
    void SetViewport(GraphicsContext *graphicsContext, int width, int height) override;
    void UpdatePushConstant(GraphicsContext *graphicsContext, GraphicsPipeline* graphicsPipeline, Shader *shader, std::string name, const void *data) override;
    void DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy);
    
};
