#include "Renderer/RenderGraph/Actions/CommandPoolAction.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/CommandPool.h"
#include "Renderer/RenderGraph/RenderGraph.h"

bool CommandPoolAction::Execute() {
    if(!_commandPool) {
        return false;
    }
    
    switch (_commandPoolAction) {
        case CPA_Empty: return false;
        case CPA_Allocate: _commandPool->AllocateCommandPool(); break;
        case CPA_Reset: _commandPool->ResetCommandPool(); break;
        case CPA_Submit: _commandPool->SubmitCommands(_submitCommandsParams); break;

        // Command Buffers
        case CPA_AllocateCommandBuffer: _commandPool->AllocateCommandBuffer(); break;
        case CPA_EnableCommandBufferRecording: _commandPool->EnableCommandBufferRecording(); break;
        case CPA_DisableCommandBufferRecording: _commandPool->DisableCommandBufferRecording(); break;
        case CPA_ReleaseCommandBuffer: _commandPool->ReleaseCommandBuffer(); break;
    }

    return true;
}
