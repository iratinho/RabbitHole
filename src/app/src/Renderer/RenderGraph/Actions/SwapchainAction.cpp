#include "Renderer/RenderGraph/Actions/SwapchainAction.hpp"
#include "Renderer/Swapchain.hpp"

SwapchainAction::SwapchainAction(const std::any &actionData) {
    IGraphAction::_actionData = actionData;
}

bool SwapchainAction::Execute() {
    if(SwapchainRequestImageData* data = std::any_cast<SwapchainRequestImageData>(&_actionData)) {
        data->_swapchain->RequestNewPresentableImage(data->_index);
        return true;
    }

    return false;
}


