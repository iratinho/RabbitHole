#include "Renderer/RenderGraph/GraphBuilder.hpp"

GraphBuilder::GraphBuilder(RenderGraph* render_graph, std::string  identifier)
    : _renderGraph(render_graph)
    , _graphIdentifier(std::move(identifier)) {
}

bool GraphBuilder::Execute() {
    for (auto& graph_action : _graphActions) {
        if(!graph_action.Execute()) {
            continue;
        }
    }

    return true;
}

void GraphBuilder::AcquirePresentableSurface(uint32_t index) {
    SwapchainRequestImageData actionData {};
    actionData._swapchain = _renderGraph->GetRenderContext()->GetSwapchain();
    actionData._index = index;
    _graphActions.emplace_back(SwapchainAction(actionData));
}

void GraphBuilder::AllocateCommandPool(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData {};
    actionData._action = ECommandPoolAction::CPA_Allocate;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::ResetCommandPool(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData {};
    actionData._action = ECommandPoolAction::CPA_Reset;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::AllocateCommandBuffer(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData{};
    actionData._action = ECommandPoolAction::CPA_AllocateCommandBuffer;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::SubmitCommands(CommandPool* commandPool, const SubmitCommandParams& submitCommandParams) {
    CommandPoolSubmitActionData actionData{};
    actionData._submitParams = submitCommandParams;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::EnableCommandBufferRecording(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData{};
    actionData._action = ECommandPoolAction::CPA_EnableCommandBufferRecording;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::DisableCommandBufferRecording(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData{};
    actionData._action = ECommandPoolAction::CPA_DisableCommandBufferRecording;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::ReleaseCommandBuffer(CommandPool* commandPool) {
    CommandPoolGenericActionData actionData{};
    actionData._action = ECommandPoolAction::CPA_ReleaseCommandBuffer;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(CommandPoolAction(actionData));
}

void GraphBuilder::AllocateFence(Fence* fence) {
    FenceGenericActionData actionData {};
    actionData._fence = fence;
    actionData._fenceAction = EFenceAction::FA_Allocate;
    _graphActions.emplace_back(FenceAction(actionData));
}

void GraphBuilder::WaitFence(Fence* fence) {
    FenceGenericActionData actionData {};
    actionData._fence = fence;
    actionData._fenceAction = EFenceAction::FA_Wait;
    _graphActions.emplace_back(FenceAction(actionData));
}

void GraphBuilder::ResetFence(Fence* fence) {
    FenceGenericActionData actionData {};
    actionData._fence = fence;
    actionData._fenceAction = EFenceAction::FA_Reset;
    _graphActions.emplace_back(FenceAction(actionData));
}

void GraphBuilder::Present(Surface* surface, const SurfacePresentParams& presentParams) {
    SurfacePresentActionData actionData;
    actionData._surface = surface;
    actionData._surfacePresentParams = presentParams;
    _graphActions.emplace_back(SurfaceAction(actionData));
}

void GraphBuilder::AllocateSurface(Surface* surface, const SurfaceCreateParams& params) {
    SurfaceAllocateActionData actionData;
    actionData._surface = surface;
    actionData._surfaceCreateParams = params;
    _graphActions.emplace_back(SurfaceAction(actionData));
}

void GraphBuilder::CopyGeometryData(Buffer* buffer, const MeshNode* meshNode) {
    BufferStageGeometryDataActionData actionData;
    actionData._buffer = buffer;
    actionData._meshNode = meshNode;
    actionData._renderContext = _renderGraph->GetRenderContext();
    _graphActions.emplace_back(BufferAction(actionData));
}

void GraphBuilder::UploadBufferData(Buffer* buffer, CommandPool* commandPool) {
    BufferUploadActionData actionData;
    actionData._buffer = buffer;
    actionData._commandPool = commandPool;
    _graphActions.emplace_back(BufferAction(actionData));
}

void GraphBuilder::AddPass(RenderContext* renderContext, CommandPool* commandPool, const RenderPassGenerator& generator, unsigned int frameIndex, const std::string& passIdentifier) {
    RenderPassActionData actionData;
    actionData._renderContext = renderContext;
    actionData._commandPool = commandPool;
    actionData._generator = generator;
    actionData._frameIndex = frameIndex;
    actionData._passIdentifier = std::move(passIdentifier);
    _graphActions.emplace_back(RenderPassActionNew(actionData));
}

