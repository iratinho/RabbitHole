#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"

class Fence;

enum EFenceAction
{
    FA_Empty,
    FA_Allocate,
    FA_Wait,
    FA_Reset
};

struct FenceActionData {
    Fence* _fence;
};

struct FenceGenericActionData : public FenceActionData {
    EFenceAction _fenceAction;
};

class FenceAction : public IGraphAction
{
public:
    FenceAction() = delete;
    explicit FenceAction(const std::any& actionData);
    ~FenceAction() override = default;
    bool Execute() override;

    EFenceAction _fenceAction;
    Fence* _fence{};
};
