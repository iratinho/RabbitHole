#pragma once
#include "Core/Utils.hpp"
#include "GPUDefinitions.h"
#include "GraphBuilder.hpp"

struct InitializationParams;
class GraphicsContext;
class GraphicsPipeline;
class Scene;
class RenderPass;
class RenderContext;
class Window;

class RenderSystemV2 {
public:
    RenderSystemV2();
    ~RenderSystemV2();
    
    bool Initialize(Window* window);
    bool Process(Scene* scene);    
    
    static void RegisterRenderPass(RenderPass* pass);

private:
    void BeginFrame(GraphicsContext* graphicsContext, Scene* scene);
    void Render(GraphicsContext* graphicsContext, Scene* scene);
    void EndFrame(GraphicsContext* graphicsContext);

    static std::vector<RenderPass*>& GetRenderPasses() {
        static std::vector<RenderPass*> renderPasses;
        return renderPasses;
    };
    
private:    
    GraphBuilder _graphBuilder;
    std::map<Window*, uint8_t> _windowsContexts;
};
