#pragma once

#include "Renderer/RenderPass/RenderPassInterface.hpp"

class GraphicsContext;
class GraphBuilder;
class Scene;

class PhongRenderPass : public IRenderPass {
public:
    bool Setup(GraphBuilder* graphBuilder, GraphicsContext* graphicsContext, Scene* scene) override;
};
