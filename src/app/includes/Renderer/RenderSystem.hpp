#pragma once
#include "Core/Components/SceneComponent.hpp"
#include "Renderer/render_context.hpp"
#include "entt/entt.hpp"
#include "RenderGraph/Actions/SurfaceAction.hpp"

class Surface;
class CommandPool;
class RenderTarget;
class RenderGraph;
class RenderContext;
class IRenderer;
class Fence;
class GraphBuilder;

struct PresistentRenderTargets {
    RenderTarget* scene_color_render_target;
    RenderTarget* scene_depth_render_target;
};

struct SyncPrimitives {
    VkFence in_flight_fence;
    VkSemaphore render_finish_semaphore;
    std::shared_ptr<Fence> in_flight_fence_new;
};

struct FrameData {
    SyncPrimitives sync_primitives;
    std::shared_ptr<CommandPool> _commandPool;
    std::shared_ptr<Surface> _presentableSurface;
};
    
class RenderSystem {
public:
    ~RenderSystem();
    
    bool Initialize(InitializationParams initialization_params);
    bool Process(const entt::registry& registry);
    void HandleResize(int width, int height);

    uint32_t GetCurrentFrameIndex() { return _frameIndex; };

    RenderContext* GetRenderContext() const { return _renderContext; }
    RenderGraph* GetRenderGraph() const { return _renderGraph; }

    // todo IMRPOVE THIS
    bool ReleaseResources();
    
private:
    void AllocateGeometryBuffers(const entt::registry& registry, GraphBuilder* graphBuilder, unsigned frameIndex);
    bool CreateSyncPrimitives();

    InitializationParams m_InitializationParams;

    RenderContext* _renderContext;
    RenderGraph* _renderGraph;

    std::vector<FrameData> _frameData;
    uint32_t _frameIndex = 0;

    bool needs_swapchain_recreation = false;
    bool invalid_surface_for_swapchain = false;
};
