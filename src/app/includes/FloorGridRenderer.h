#pragma once
#include <render_context.h>

namespace app::renderer {
    class FloorGridRenderer : public IRenderer {
    public:
        FloorGridRenderer() = default;
        bool Initialize(RenderContext* const render_context, const InitializationParams& initialization_params) override;
        VkCommandBuffer RecordCommandBuffers(uint32_t idx) override;
        bool AllocateFrameBuffers(int command_idx) override;
        bool AllocateCommandBuffers(VkCommandPool command_pool, int pool_idx) override;
        bool AllocateRenderingResources() override;
        void HandleResize(int width, int height) override;
        
    private:
        bool CreateRenderPass();
        bool CreateGraphicsPipeline();
        bool CreateCommandPool();
        bool CreateCommandBuffer();
        bool CreateRenderingResources();
        bool CreateRenderingBuffers();
        
        RenderContext* render_context_;
        VkRenderPass render_pass_;
        VkPipelineLayout pipeline_layout_;
        std::vector<VkPipeline> pipelines_;
        VkCommandPool command_pool_;
        VkCommandBuffer command_buffer_;
        VkImageView color_image_view;
        VkImage color_image;
        VkImageView depth_image_view_;
        VkImage depth_image_;
        VkFramebuffer framebuffer;

        IndexRenderingData plane_rendering_data_;
    };    
}
