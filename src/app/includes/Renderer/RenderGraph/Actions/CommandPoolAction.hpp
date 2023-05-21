#pragma once
#include "Renderer/CommandPool.hpp"
#include "Renderer/RenderPass/RenderPass.hpp"

class CommandPool;
class CommandBuffer;

enum ECommandPoolAction
{
    CPA_Empty,
    CPA_Allocate,
    CPA_Reset,

    // Command Buffers
    CPA_AllocateCommandBuffer,
    CPA_EnableCommandBufferRecording,
    CPA_DisableCommandBufferRecording,
    CPA_ReleaseCommandBuffer
};

struct CommandPoolActionData {
    CommandPool* _commandPool;
};

struct CommandPoolGenericActionData : public CommandPoolActionData {
    ECommandPoolAction _action;
};

struct CommandPoolSubmitActionData : public CommandPoolActionData {
    SubmitCommandParams _submitParams;
};

class CommandPoolAction : public IGraphAction
{
public:
    CommandPoolAction(const std::any& actionData);
    ~CommandPoolAction() override = default;
    bool Execute() override;
};
