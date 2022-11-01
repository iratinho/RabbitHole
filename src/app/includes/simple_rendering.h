#pragma once
#include "render_context.h"
#include "vulkan/vulkan_core.h"

namespace app::window {
    class Window;
}

namespace app::renderer {
    struct Position {
        float x;
        float y;
    };

    struct Color {
        float r;
        float g;
        float b;
    };

    struct VertexData {
        Position position;
        Color color;
    };
    
    struct SyncPrimitives {
        VkFence in_flight_fence;
        VkSemaphore swapchain_image_semaphore;
        VkSemaphore render_finish_semaphore;
    };
    
    class SimpleRendering {
        enum SemaphoresIdentifiers {
            swapchain_acquire
        };
    public:
        SimpleRendering() = default;
        bool Initialize(const InitializationParams& initialization_params);
        bool Draw();
        void HandleResize(int width, int height);
    private:
        bool CreateRenderPass();
        bool CreateGraphicsPipeline();
        bool CreateSwapchainFramebuffers();
        bool CreateCommandPool();  // Later this will be abstracted to a Frame class
        bool CreateCommandBuffers();
        bool RecordCommandBuffers(VkFramebuffer target_swapchain_framebuffer);
        bool CreateSyncObjects();
        bool RecreateSwapchain();
        bool CreateRenderingBuffers();
        
        RenderContext* render_context_;
        VkRenderPass render_pass_;
        VkCommandPool command_pool_;
        VkBuffer triangle_vertex_buffer_;

        std::vector<VkPipeline> pipelines_;
        std::vector<VkFramebuffer> swapchain_framebuffers;

        // entry indice off this vector's is indexing the current_frame_index
        std::vector<SyncPrimitives> syncrhonization_primitive_;
        std::vector<VkCommandBuffer> command_buffers_;

        int current_frame_index = 0;
        bool needs_swapchain_recreation = false;
        bool invalid_surface_for_swapchain = false;

        window::Window* window_;
    };    
}

