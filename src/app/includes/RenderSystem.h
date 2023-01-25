#pragma once
#include "render_context.h"

class RenderGraph;
class RenderContext;
class IRenderer;

struct PresistentRenderTargets {
    RenderTarget* scene_color_render_target;
    RenderTarget* scene_depth_render_target;
};

struct SyncPrimitives {
    VkFence in_flight_fence;
    VkSemaphore swapchain_image_semaphore;
    VkSemaphore render_finish_semaphore;
};

struct FrameData {
    SyncPrimitives sync_primitives;
    VkCommandPool command_pool;
};
    
class RenderSystem {
public:
    ~RenderSystem();
    
    bool Initialize(InitializationParams initialization_params);
    bool Process();
    void HandleResize(int width, int height);
        
private:
    bool CreateSwapchainRenderTargets();
    bool CreateRenderingResources();
    bool CreateSyncPrimitives();
    bool RecreateSwapchain();

    RenderContext* render_context_;
    RenderGraph* render_graph_;

    std::vector<FrameData> frame_data_;
    uint32_t frame_idx = 0;

    bool needs_swapchain_recreation = false;
    bool invalid_surface_for_swapchain = false;
};
