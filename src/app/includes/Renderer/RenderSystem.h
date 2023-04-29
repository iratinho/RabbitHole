#pragma once
#include "Renderer/render_context.h"
#include "Renderer/Fence.h"
#include "entt/entt.hpp"

class RenderTarget;
class RenderGraph;
class RenderContext;
class IRenderer;

struct PresistentRenderTargets {
    RenderTarget* scene_color_render_target;
    RenderTarget* scene_depth_render_target;
};

struct SyncPrimitives {
    VkFence in_flight_fence;
    VkSemaphore render_finish_semaphore;
    std::unique_ptr<Fence> in_flight_fence_new;
};

struct FrameData {
    SyncPrimitives sync_primitives;
};
    
class RenderSystem {
public:
    ~RenderSystem();
    
    bool Initialize(InitializationParams initialization_params);
    bool Process(const entt::registry& registry);
    void HandleResize(int width, int height);

    uint32_t GetCurrentFrameIndex() { return frame_idx; };

    RenderContext* GetRenderContext() { return render_context_; }

    // todo IMRPOVE THIS
    bool ReleaseResources();
    
private:
    bool CreateSyncPrimitives();

    InitializationParams m_InitializationParams;

    RenderContext* render_context_;
    RenderGraph* render_graph_;

    std::vector<FrameData> frame_data_;
    uint32_t frame_idx = 0;

    bool needs_swapchain_recreation = false;
    bool invalid_surface_for_swapchain = false;
};
