#pragma once
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include "Core/Utils.hpp"

class RenderContext;
class GraphBuilder;
class RenderTarget;

struct PassResource {
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
};

struct PipelineStateObject {
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkFramebuffer framebuffer = nullptr;
    VkDescriptorSet descriptorSet;
    // std::unordered_map<std::string, VkFramebuffer> framebuffers; // This still looks strange... Because PSO have a unique identifier they are always the same for every frame. Mabye the framebuffers and command buffers dont belong to the PSO and we need to regiter them with the render graph instead
    
    // VkCommandBuffer command_buffer = nullptr; // need this per swapchain image?
};

class RenderGraph {
public:
    RenderGraph(RenderContext* render_context);
    
    GraphBuilder MakeGraphBuilder(const std::string& graph_identifier);

    RenderContext* GetRenderContext() { return render_context_; }

    void RegisterRenderTarget(std::string identifier, RenderTarget* render_target);

    void ReleaseRenderTargets();

    void ReleasePassResources();

    RenderTarget* GetRenderTarget(std::string identifier);
    
    PipelineStateObject* RegisterPSO(const std::string& identifier, PipelineStateObject pso);
    PipelineStateObject* GetCachedPSO(const std::string& identifier);

    PipelineStateObject* RegisterPSO2(const std::string& identifier, PipelineStateObject pso);
    // Be careful with this, behind it uses a circular buffer that for each access will advance it
    PipelineStateObject* GetCachedPSO2(const std::string& identifier);
    PipelineStateObject* GetCachedPSO3(const std::string& identifier);

    void RegisterPassResource(const std::string& identifier, const PassResource& pass_resource);
    PassResource* GetCachedPassResource(const std::string& identifier);
    
private:
    RenderContext* render_context_;
    std::unordered_map<std::string, VkPipeline> cached_pipelines_;
    std::unordered_map<std::string, PipelineStateObject> cached_pso_;
    std::unordered_map<std::string, CircularBuffer<PipelineStateObject, 2>> _cachedPsoNew;
    std::unordered_map<std::string, RenderTarget*> render_targets_;
    std::unordered_map<std::string, VkCommandPool> cached_pools_;
    std::unordered_map<std::string, PassResource> cached_pass_resources_;
};
