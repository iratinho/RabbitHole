#pragma once
#include "Renderer/RenderPass/RenderPass.h"

class Fence;

enum EFenceAction
{
    FA_Empty,
    FA_Allocate,
    FA_Wait,
    FA_Reset
};

class FenceAction : public IGraphAction
{
public:
    ~FenceAction() override = default;
    bool Execute() override;

    EFenceAction _fenceAction;
    Fence* _fence;
};
