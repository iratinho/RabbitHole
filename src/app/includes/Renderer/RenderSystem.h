#pragma once
#include "Core/Components/SceneComponent.h"
#include "Renderer/render_context.h"
#include "entt/entt.hpp"
#include "RenderGraph/Actions/SurfaceAction.h"

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

    uint32_t GetCurrentFrameIndex() { return frame_idx; };

    RenderContext* GetRenderContext() const { return render_context_; }
    RenderGraph* GetRenderGraph() const { return render_graph_; }

    // todo IMRPOVE THIS
    bool ReleaseResources();
    
private:
    void AllocateGeometryBuffers(const entt::registry& registry, GraphBuilder* graphBuilder, unsigned frameIndex);
    bool CreateSyncPrimitives();

    InitializationParams m_InitializationParams;

    RenderContext* render_context_;
    RenderGraph* render_graph_;

    std::vector<FrameData> frame_data_;
    uint32_t frame_idx = 0;

    bool needs_swapchain_recreation = false;
    bool invalid_surface_for_swapchain = false;
};
