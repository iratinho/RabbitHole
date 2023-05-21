#include "Renderer/RenderGraph/Actions/CommandPoolAction.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/CommandPool.hpp"
#include "Renderer/RenderGraph/RenderGraph.hpp"

CommandPoolAction::CommandPoolAction(const std::any& actionData) {
    IGraphAction::_actionData = std::move(actionData);
}

bool CommandPoolAction::Execute() {
    // Submit
    if(CommandPoolSubmitActionData* data = std::any_cast<CommandPoolSubmitActionData>(&_actionData)) {
        if(data->_commandPool) {
            data->_commandPool->SubmitCommands(data->_submitParams);
            return true;
        }
    }
    
    // Generic Actions with no parameters to pass around
    if(CommandPoolGenericActionData* data = std::any_cast<CommandPoolGenericActionData>(&_actionData)) {
        if(data->_commandPool) {
            switch (data->_action) {
                case CPA_Empty:
                    break;
                case CPA_Allocate:
                    data->_commandPool->AllocateCommandPool();
                    return true;
                    break;
                case CPA_Reset:
                    data->_commandPool->ResetCommandPool();
                    return true;
                    break;
                case CPA_AllocateCommandBuffer:
                    data->_commandPool->AllocateCommandBuffer();
                    return true;
                    break;
                case CPA_EnableCommandBufferRecording:
                    data->_commandPool->EnableCommandBufferRecording();
                    return true;
                    break;
                case CPA_DisableCommandBufferRecording:
                    data->_commandPool->DisableCommandBufferRecording();
                    return true;
                    break;
                case CPA_ReleaseCommandBuffer:
                    data->_commandPool->ReleaseCommandBuffer();
                    return true;
                    break;
                default:
                    break;
            }
        }
    }

    return false;
}
