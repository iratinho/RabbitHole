#pragma once

#include "Renderer/RenderPass/RenderPassInterface.hpp"

class GraphicsContext;
class GraphBuilder;
class Scene;

class MatcapRenderPass : public IRenderPass {
public:
    bool Setup(GraphBuilder* graphBuilder, GraphicsContext* graphicsContext, Scene* scene) override;
    
    std::string GetIdentifier() override { return "MatCapRenderPass"; };
};
