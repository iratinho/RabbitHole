#pragma once
#include "Renderer/RenderPass/RenderPass.h"

template <typename RenderPassDesc>
class RenderPassAction final : public IGraphAction
{
public:
    bool Execute() override
    {
        RenderPassDesc::PassType(_renderGraph, _passDescription, _graphIdentifier).Execute();
        return true;
    }
    
    RenderPassDesc* _passDescription;
    RenderGraph* _renderGraph {};
    std::string _graphIdentifier;
};
