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
class IRenderPass;

class RenderSystemV2 {
public:
    RenderSystemV2();
    ~RenderSystemV2();
    
    bool Initialize(const InitializationParams& params);
    bool Process(Scene* scene);    
    
    static void RegisterRenderPass(IRenderPass* pass);

private:
    void BeginFrame(Scene* scene);
    void Render(Scene* scene);
    void EndFrame();

    static std::vector<IRenderPass*>& GetRenderPasses() {
        static std::vector<IRenderPass*> renderPasses;
        return renderPasses;
    };
    
private:
    std::shared_ptr<Device> _device;
    std::vector<std::shared_ptr<GraphicsContext>> _graphicsContext;
    std::unique_ptr<TransferContext> _transferContext;
    
    GraphBuilder _graphBuilder;
    
    int currentContext = 0;
};
