#include "Renderer/RenderGraph/Actions/RenderPassAction.hpp"
#include "Renderer/RenderPass/RenderPassExecutor.hpp"

RenderPassActionNew::RenderPassActionNew(const std::any &actionData) {
    IGraphAction::_actionData = actionData;
}

bool RenderPassActionNew::Execute() {
    if(RenderPassActionData* data = std::any_cast<RenderPassActionData>(&_actionData)) {
        if(data->_renderContext) {
            RenderPassExecutor(data->_renderContext, data->_commandPool, std::move(data->_generator), data->_passIdentifier).Execute(data->_frameIndex);
        }
    }

    return false;
}
