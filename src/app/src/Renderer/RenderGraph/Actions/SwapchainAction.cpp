#include "Renderer/RenderGraph/Actions/SwapchainAction.hpp"

#include "Renderer/Swapchain.hpp"

bool SwapchainAction::Execute()
{
    switch (_swapchainAction) {
        case SCA_Empty: return false;;
        case SCA_RequestImage: _swapchain->RequestNewPresentableImage(_index); break;
    }

    return true;
}
