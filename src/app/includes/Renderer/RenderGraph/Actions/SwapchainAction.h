#pragma once
#include "Renderer/RenderPass/RenderPass.h"

enum ESwapchainAction
{
    SCA_Empty,
    SCA_RequestImage
};

class SwapchainAction : public IGraphAction
{
public:
    ~SwapchainAction() override = default;
    bool Execute() override;

    ESwapchainAction _swapchainAction;
    Swapchain* _swapchain;
    unsigned int _index;
};