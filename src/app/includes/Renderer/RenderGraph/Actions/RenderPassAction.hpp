#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"
#include "Renderer/RenderPass/RenderPassGenerator.hpp"

class CommandPool;

template <typename RenderPassDesc>
class RenderPassAction final : public IGraphAction
{
public:
    bool Execute() override
    {
        typename RenderPassDesc::PassType(_renderGraph, _passDescription, _graphIdentifier).Execute();
        return true;
    }
    
    RenderPassDesc* _passDescription;
    RenderGraph* _renderGraph {};
    std::string _graphIdentifier;
};

struct RenderPassActionData {
    RenderContext* _renderContext = nullptr;
    CommandPool* _commandPool;
    RenderPassGenerator _generator;
    unsigned int _frameIndex;
    std::string _passIdentifier;
};

class RenderPassActionNew : public IGraphAction {
public:
    RenderPassActionNew() = delete;
    RenderPassActionNew(const std::any& actionData);
    bool Execute() override;
};
