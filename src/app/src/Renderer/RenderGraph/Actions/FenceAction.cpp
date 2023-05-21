#include "Renderer/RenderGraph/Actions/FenceAction.hpp"
#include "Renderer/Fence.hpp"

FenceAction::FenceAction(const std::any &actionData) {
    IGraphAction::_actionData = actionData;
}

bool FenceAction::Execute() {
    if(FenceGenericActionData* data = std::any_cast<FenceGenericActionData>(&_actionData)) {
        if(data->_fence) {
            switch (data->_fenceAction) {
                case FA_Empty: break;
                case FA_Allocate:
                    data->_fence->AllocateFence();
                    return true;
                case FA_Wait:
                    data->_fence->WaitFence();
                    return true;
                case FA_Reset:
                    data->_fence->ResetFence();
                    return true;
                default: ;
            }
        }
    }

    return false;
}
