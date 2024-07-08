#pragma once
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/RenderSystemV2.hpp"

class GraphicsContext;
class GraphBuilder;
class Scene;

class FloorGridRenderPass : public IRenderPass {
public:
    bool Setup(GraphBuilder* graphBuilder, GraphicsContext* graphicsContext, Scene* scene) override;
};