#include "Renderer/RenderGraph/GraphBuilder.h"

GraphBuilder::GraphBuilder(RenderGraph* render_graph, const std::string& identifier)
    : _renderGraph(render_graph)
    , _graphIdentifier(identifier) {
}

bool GraphBuilder::Execute() {
    for (auto& graph_action : _graphActions) {
        if(!graph_action.Execute()) {
            continue;
        }
    }

    return true;
}

void GraphBuilder::AllocateCommandPool(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_Allocate;
    action._renderGraph = _renderGraph;
    action._commandPool = commandPool;
    _graphActions.emplace_back(action);
}

void GraphBuilder::AcquirePresentableSurface(uint32_t index) {
    SwapchainAction action;
    action._swapchainAction = SCA_RequestImage;
    action._swapchain = _renderGraph->GetRenderContext()->GetSwapchain();
    action._index = index;
    _graphActions.emplace_back(action);
}

void GraphBuilder::AllocateCommandBuffer(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_AllocateCommandBuffer;
    action._commandPool = commandPool;
    action._renderGraph = _renderGraph;
    _graphActions.emplace_back(action);
}

void GraphBuilder::EnableCommandBufferRecording(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_EnableCommandBufferRecording;
    action._commandPool = commandPool;
    action._renderGraph = _renderGraph;
    _graphActions.emplace_back(action);
}

void GraphBuilder::DisableCommandBufferRecording(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_DisableCommandBufferRecording;
    action._commandPool = commandPool;
    action._renderGraph = _renderGraph;
    _graphActions.emplace_back(action);
}

void GraphBuilder::ReleaseCommandBuffer(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_ReleaseCommandBuffer;
    action._commandPool = commandPool;
    action._renderGraph = _renderGraph;
    _graphActions.emplace_back(action);
}

void GraphBuilder::AllocateFence(const std::shared_ptr<Fence>& fence) {
    FenceAction action;
    action._fenceAction = FA_Allocate;
    action._fence = fence.get();
    _graphActions.emplace_back(action);
}

void GraphBuilder::WaitFence(Fence* fence) {
    FenceAction action;
    action._fenceAction = FA_Wait;
    action._fence = fence;
    _graphActions.emplace_back(action);
}

void GraphBuilder::ResetFence(Fence* fence) {
    FenceAction action;
    action._fenceAction = FA_Reset;
    action._fence = fence;
    _graphActions.emplace_back(action);
}

void GraphBuilder::ResetCommandPool(CommandPool* commandPool) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_Reset;
    action._commandPool = commandPool;
    action._renderGraph = _renderGraph;
    _graphActions.emplace_back(action);
}

void GraphBuilder::SubmitCommands(CommandPool* commandPool, const SubmitCommandParams& submitCommandParams) {
    CommandPoolAction action;
    action._commandPoolAction = CPA_Submit;
    action._commandPool = commandPool;
    action._submitCommandsParams = submitCommandParams;
    _graphActions.emplace_back(action);
}

void GraphBuilder::Present(const std::shared_ptr<Surface>& surface, const SurfacePresentParams& presentParams) {
    SurfaceAction action;
    action._surfaceAction = SA_Present;
    action._surfacePresentParams = presentParams;
    action._surface = surface;
    _graphActions.emplace_back(action);
}

void GraphBuilder::AllocateSurface(const std::shared_ptr<Surface>& surface, SurfaceCreateParams& params) {
    SurfaceAction action;
    action._surfaceAction = SA_Allocate;
    action._surfaceCreateParams = params;
    action._surface = surface;
    _graphActions.emplace_back(action);
}

void GraphBuilder::CopyGeometryData(std::shared_ptr<Buffer> buffer, const MeshNode* meshNode) {
    BufferAction action;
    action._bufferAction = EBufferAction::BA_StageGeometryData;
    action._renderContext = _renderGraph->GetRenderContext();
    action._buffer = std::move(buffer);
    action._meshNode = meshNode;
    
    _graphActions.emplace_back(action);
}

void GraphBuilder::UploadBufferData(std::shared_ptr<Buffer> buffer, CommandBuffer* commandBuffer) {
    BufferAction action;
    action._bufferAction = EBufferAction::BA_TransferToGPU;
    action._renderContext = _renderGraph->GetRenderContext();
    action._buffer = std::move(buffer);
    action._commandBuffer = commandBuffer;
    
    _graphActions.emplace_back(action);
}
