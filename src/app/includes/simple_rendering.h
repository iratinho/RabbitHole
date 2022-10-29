#pragma once
#include "render_context.h"
#include "vulkan/vulkan_core.h"

namespace app::renderer {
    class SimpleRendering {
        enum SemaphoresIdentifiers {
            swapchain_acquire
        };
    public:
        SimpleRendering() = default;
        bool Initialize(const InitializationParams& initialization_params);
        bool Draw();
    private:
        bool CreateRenderPass();
        bool CreateGraphicsPipeline();
        bool CreateSwapchainFramebuffers();
        bool CreateCommandPool();  // Later this will be abstracted to a Frame class
        bool CreateCommandBuffers();
        bool RecordCommandBuffers(VkFramebuffer target_swapchain_framebuffer);
        bool CreateSyncObjects();
        
        RenderContext* render_context_;
        VkRenderPass render_pass_;
        VkCommandPool command_pool_;
        VkCommandBuffer command_buffer_;
        VkSemaphore swapchain_image_acquire_semaphore_;
        std::vector<VkPipelineStageFlags> wait_sages_flags_;
        VkSemaphore render_finished_semaphore;
        VkFence frame_end_fence_;
        
        std::vector<VkPipeline> pipelines_;
        std::vector<VkFramebuffer> swapchain_framebuffers;
    };    
}

