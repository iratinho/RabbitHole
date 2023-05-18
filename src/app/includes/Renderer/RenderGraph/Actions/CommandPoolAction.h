#pragma once
#include "Renderer/CommandPool.h"
#include "Renderer/RenderPass/RenderPass.h"

class CommandPool;
class CommandBuffer;

enum ECommandPoolAction
{
    CPA_Empty,
    CPA_Allocate,
    CPA_Reset,
    CPA_Submit,

    // Command Buffers
    CPA_AllocateCommandBuffer,
    CPA_EnableCommandBufferRecording,
    CPA_DisableCommandBufferRecording,
    CPA_ReleaseCommandBuffer
};

class CommandPoolAction : public IGraphAction
{
public:
    ~CommandPoolAction() override = default;
    bool Execute() override;

    ECommandPoolAction _commandPoolAction;
    RenderGraph* _renderGraph;
    CommandPool* _commandPool;

    // Only used when using CPA_Submit
    SubmitCommandParams _submitCommandsParams;
};