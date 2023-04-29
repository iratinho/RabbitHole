#include "Renderer/RenderGraph/RenderGraph.h"
#include "Renderer/RenderGraph/GraphBuilder.h"
#include "Renderer/RenderTarget.h"

RenderGraph::RenderGraph(RenderContext* render_context)
    : render_context_(render_context)
    , m_commandBufferManager(new CommandBufferManager(render_context)){
}

void RenderGraph::RegisterRenderTarget(std::string identifier, RenderTarget* render_target) {
    render_targets_[identifier] = render_target;
}

void RenderGraph::ReleaseRenderTargets() {
    for (auto[identifier, render_target]  : render_targets_) {
        delete render_target;
    }

    render_targets_.clear();
}

void RenderGraph::ReleasePassResources() {
    for (auto&[identifier, pass_resource] : cached_pass_resources_) {
        render_context_->DestroyFrameBuffer(pass_resource.framebuffer);
    }

    for (auto&[identifier, pool] : cached_pools_) {
        render_context_->DestroyCommandPool(pool);
    }

    cached_pass_resources_.clear();
    cached_pools_.clear();
}

RenderTarget* RenderGraph::GetRenderTarget(std::string identifier) {
    if(!render_targets_.empty() && render_targets_.find(identifier) != render_targets_.end()) {
        return render_targets_[identifier];
    }

    return nullptr;
}

GraphBuilder RenderGraph::MakeGraphBuilder(const std::string& graph_identifier) {
    return {this, graph_identifier};
}

PipelineStateObject* RenderGraph::RegisterPSO(const std::string& identifier, PipelineStateObject pso) {
    cached_pso_[identifier] = pso;
    return &cached_pso_[identifier];
}

PipelineStateObject* RenderGraph::GetCachedPSO(const std::string& identifier) {
    if(!cached_pso_.empty() && cached_pso_.find(identifier) != cached_pso_.end()) {
        return &cached_pso_[identifier];
    }

    return nullptr;
}

PipelineStateObject* RenderGraph::RegisterPSO2(const std::string& identifier, PipelineStateObject pso) {
    _cachedPsoNew[identifier].push(pso);
    return &_cachedPsoNew[identifier].getCurrent();
}

PipelineStateObject* RenderGraph::GetCachedPSO2(const std::string& identifier)
{
    if(!_cachedPsoNew.empty() && _cachedPsoNew.find(identifier) != _cachedPsoNew.end()) {
        return &_cachedPsoNew[identifier].peek();
    }
    
    return nullptr;
}

void RenderGraph::RegisterPassResource(const std::string& identifier, const PassResource& pass_resource)
{
    cached_pass_resources_[identifier] = pass_resource;
}

PassResource* RenderGraph::GetCachedPassResource(const std::string& identifier)
{
    if(!cached_pass_resources_.empty() && cached_pass_resources_.find(identifier) != cached_pass_resources_.end()) {
        return &cached_pass_resources_[identifier];
    }

    return nullptr;
}
