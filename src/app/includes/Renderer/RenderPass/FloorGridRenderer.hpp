#pragma once
#include "render_context.h"

class FloorGridRenderer : public IRenderer {
public:
    FloorGridRenderer() = default;
    bool Initialize(RenderContext* const render_context, const InitializationParams& initialization_params) override;
    VkCommandBuffer RecordCommandBuffers(uint32_t idx) override;
    bool AllocateFrameBuffers(int command_idx, struct PresistentRenderTargets render_targets) override;
    bool AllocateCommandBuffers(VkCommandPool command_pool, int pool_idx) override;
    bool AllocateRenderingResources() override;
    void HandleResize(int width, int height) override;
        
private:
    bool CreateRenderPass();
    bool CreateGraphicsPipeline();
    bool CreateCommandPool();
    bool CreateCommandBuffer();
    bool CreateRenderingResources();
        
    RenderContext* render_context_;
    VkRenderPass render_pass_;
    VkPipelineLayout pipeline_layout_;
    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;
    VkImageView color_image_view;
    VkImage color_image;
    VkImageView depth_image_view_;
    VkImage depth_image_;
    VkFramebuffer framebuffer;
    app::window::Window* window_;
    std::vector<VkCommandBuffer> command_buffers_;
    std::vector<VkFramebuffer> framebuffers_;
    VkPipeline pipeline_;
    
    IndexRenderingData plane_rendering_data_;
};    
