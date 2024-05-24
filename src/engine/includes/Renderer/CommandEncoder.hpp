#pragma once
#include "Renderer/GPUDefinitions.h"
#include "Components/PrimitiveProxyComponent.hpp"

class GraphicsContext;
class GraphicsPipeline;
class RenderContext;
class Shader;


class CommandEncoder {
public:
    virtual ~CommandEncoder() = default;
    
    static std::unique_ptr<CommandEncoder> MakeCommandEncoder(std::shared_ptr<RenderContext> renderContext);
    
    virtual void SetViewport(GraphicsContext *graphicsContext, int width, int height) = 0;
    
    virtual void UpdatePushConstants(GraphicsContext* graphicsContext, GraphicsPipeline* graphicsPipeline, Shader* shader, const void* data) = 0;
    
    virtual void DrawPrimitiveIndexed(GraphicsContext* graphicsContext, const PrimitiveProxyComponent& proxy) = 0;
        
protected:
    std::shared_ptr<RenderContext> _renderContext;
};
