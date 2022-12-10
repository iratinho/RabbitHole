#pragma once
#include "render_context.h"

namespace app::renderer {
    class OpaqueRenderer : public IRenderer {
    public:
        OpaqueRenderer() = default;

        bool Initialize(RenderContext* const render_context, const InitializationParams& initialization_params) override;
        bool AllocateCommandBuffers(VkCommandPool command_pool, int pool_idx) override;
        bool AllocateFrameBuffers(int idx) override;
        VkCommandBuffer RecordCommandBuffers(uint32_t idx) override;
        bool AllocateRenderingResources() override;
        void HandleResize(int width, int height) override;

    private:
        bool CreateRenderPass();
        bool CreateGraphicsPipeline();
        bool CreateFrameBuffers();
        
        VkRenderPass render_pass_;
        VkPipelineLayout pipeline_layout_;
        VkPipeline pipeline_;
        VkCommandPool command_pool_;
        std::vector<VkFramebuffer> framebuffers_;
        std::vector<VkCommandBuffer> command_buffers_;
        RenderContext* render_context_;
        IndexRenderingData triangle_rendering_data_;
        window::Window* window_;
    };    
}
