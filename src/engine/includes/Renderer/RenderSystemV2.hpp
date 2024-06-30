#pragma once
#include "Core/Utils.hpp"
#include "GPUDefinitions.h"
#include "GraphBuilder.hpp"

using Device = class RenderContext;
struct InitializationParams;
class GraphicsContext;
class TransferContext;
class GraphicsPipeline;
class Scene;

class RenderSystemV2 {
public:
    RenderSystemV2();
    ~RenderSystemV2();
    
    bool Initialize(const InitializationParams& params);
    bool Process(Scene* scene);
    
private:
    void BeginFrame(Scene* scene);
    void Render(Scene* scene);
    void EndFrame();
    bool SetupMatCapRenderPass(GraphicsContext* graphicsContext, Scene* scene);
    bool SetupBasePass(GraphicsContext* graphicsContext, Scene* scene);
    bool SetupFloorGridRenderPass(GraphicsContext* graphicsContext, Scene* scene);
    
private:
    std::shared_ptr<Device> _device;
//    CircularBuffer<std::shared_ptr<GraphicsContext>, 2> _graphicsContext;
    std::vector<std::shared_ptr<GraphicsContext>> _graphicsContext;
    std::unique_ptr<TransferContext> _transferContext;
    
    GraphBuilder _graphBuilder;

    
    int currentContext = 0;
};
