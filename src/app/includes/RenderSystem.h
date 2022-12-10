#pragma once
#include "render_context.h"

namespace app::renderer {
    class RenderContext;
    class IRenderer;

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
        bool Initialize(InitializationParams initialization_params);
        bool Process();
        void HandleResize(int width, int height);
        
    private:
        bool CreateRenderingResources();
        bool CreateSyncPrimitives();
        bool RecreateSwapchain();
        
        RenderContext* render_context_;
        IRenderer* floor_grid_renderer_;
        IRenderer* opaque_renderer_;
        std::vector<IRenderer*> renderers_;
        std::vector<FrameData> frame_data_;
        uint32_t frame_idx = 0;
        bool needs_swapchain_recreation = false;
        bool invalid_surface_for_swapchain = false;
    };
}
