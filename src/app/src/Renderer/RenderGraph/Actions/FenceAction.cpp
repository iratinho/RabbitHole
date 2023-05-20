#include "Renderer/RenderGraph/Actions/FenceAction.hpp"
#include "Renderer/Fence.hpp"

bool FenceAction::Execute()
{
    switch (_fenceAction) {
        case FA_Empty: return false;
        case FA_Allocate: _fence->AllocateFence(); break;
        case FA_Wait:  _fence->WaitFence(); break;
        case FA_Reset: _fence->ResetFence(); break;
    default: ;
    }

    return true;
}
