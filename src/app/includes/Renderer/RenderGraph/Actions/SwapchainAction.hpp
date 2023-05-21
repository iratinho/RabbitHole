#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"

struct SwapchainActionData {
    Swapchain* _swapchain;
};

struct SwapchainRequestImageData : public SwapchainActionData {
    unsigned int _index;
};

class SwapchainAction : public IGraphAction {
public:
    SwapchainAction() = delete;
    explicit SwapchainAction(const std::any& actionData);
    ~SwapchainAction() override = default;
    
    bool Execute() override;
};
