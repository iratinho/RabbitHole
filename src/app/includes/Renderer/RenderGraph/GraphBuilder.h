#pragma once
#include "Renderer/CommandPool.h"
#include <functional>
#include <glm.hpp>
#include "Renderer/RenderGraph/RenderGraph.h"
#include "Renderer/RenderPass/RenderPass.h"
#include "Renderer/Fence.h"
#include "Renderer/Swapchain.h"
#include "Renderer/CommandBufferManager.h"

enum ShaderType {
    None,
    VS,
    PS,
    CS
};

template <typename DerivedShaderType>
struct Shader {
public:
    using Type = DerivedShaderType;
    const char* _source = nullptr;
    ShaderType _type = None;
// public:
    // Type* _;
};

#define MAKE_VARADIC_LIST(...) __VA_OPT__

#define DECLARE_SHADER(type, sourc_, shaderType) struct type : public Shader<type> { type() { _source = sourc_; _type = shaderType; };
#define DECLARE_PARAMETER(type, name) public: type name;
#define END_SHADER_DECLARATION() };

#define DECLARE_DEFAULT_DESC_PARAMETERS() bool enabled_; int frameIndex; unsigned int previousPassIndex; unsigned int nextPassIndex;
#define DECLARE_PASS_DESC(name, vs_shader_type, ps_shader_type, pass) class name { private: vs_shader_type vs_shader_; ps_shader_type ps_shader; friend class pass; public: using PassType = pass; DECLARE_DEFAULT_DESC_PARAMETERS() 
#define DECLARE_PASS_PARAMETER(type, name) type name;
#define END_PASS_DESC_DECLARATION() };

#define MAKE_GRAPH_ACTION(name) struct name : public IGraphAction { \
    std::function<bool()> func; \
    \
    bool Execute() override { \
        return func(); \
    } \
};

MAKE_GRAPH_ACTION(RenderPassAction)
MAKE_GRAPH_ACTION(ResetFenceAction)
MAKE_GRAPH_ACTION(WaitFenceAction)
MAKE_GRAPH_ACTION(ResetCommandPoolAction)
MAKE_GRAPH_ACTION(RequestNewPresentableImageAction)
MAKE_GRAPH_ACTION(AllocateCommandBufferAction)
MAKE_GRAPH_ACTION(ReleaseCommandBufferAction)
MAKE_GRAPH_ACTION(EnableCommandBufferRecordingAction)
MAKE_GRAPH_ACTION(DisableCommandBufferRecordingAction)

class GraphBuilder {
public:
    GraphBuilder() = default;
    GraphBuilder(RenderGraph* render_graph, std::string identifier);

    bool Execute() {
        for (auto& graph_action : _graphActions) {
            if(!graph_action.Execute()) {
                return false;
            }
        }

        return true;
    }

    void AcquireSwapchainImage(uint32_t index) {
        RequestNewPresentableImageAction action;
        action.func = [this, index]() -> bool { return _renderGraph->GetRenderContext()->GetSwapchain()->RequestNewPresentableImage(index); };
        _graphActions.push_back(action);
    }

    void AllocateCommandBuffer(uint32_t index) {
        AllocateCommandBufferAction action;
        action.func = [this, index]() -> bool { return _renderGraph->GetCommandBufferManager()->AllocateCommandBuffer(index); };
        _graphActions.push_back(action);
    }

    void EnableCommandBufferRecording(uint32_t index) {
        EnableCommandBufferRecordingAction action;
        action.func = [this, index]() -> bool { return _renderGraph->GetRenderContext()->BeginCommandBuffer(_renderGraph->GetCommandBufferManager()->GetCommandBuffer((int)index)); };
        _graphActions.push_back(action);
    }

    void DisableCommandBufferRecording(uint32_t index) {
        DisableCommandBufferRecordingAction action;
        action.func = [this, index]() -> bool { return _renderGraph->GetRenderContext()->EndCommandBuffer(_renderGraph->GetCommandBufferManager()->GetCommandBuffer((int)index)); };
        _graphActions.push_back(action);
    }

    void ReleaseCommandBuffer(uint32_t index) {
        ReleaseCommandBufferAction action;
        action.func = [this, index]() -> bool { return _renderGraph->GetCommandBufferManager()->ReleaseCommandBuffer(index); };
        _graphActions.push_back(action);
    }

    void WaitFence(Fence* fence) {
        WaitFenceAction action;
        action.func = [this, fence]() -> bool { fence->WaitFence(); return true; };
        _graphActions.push_back(action);
    };

    void ResetFence(Fence* fence) {
        ResetFenceAction action;
        action.func = [this, fence]() -> bool { fence->ResetFence(); return true; };
        _graphActions.push_back(action);
    };

    void ResetCommandPool(VkCommandPool commandPool) {
        ResetCommandPoolAction action;
        action.func = [this, commandPool]() -> bool { CommandPool command_pool (_renderGraph->GetRenderContext(), commandPool); command_pool.ResetCommandPool(); return true; };
        _graphActions.push_back(action);
    }

    void ResetCommandPoolLambda(std::function<VkCommandPool()> commandPool) {
        ResetCommandPoolAction action;
        action.func = [this, commandPool]() -> bool { CommandPool command_pool (_renderGraph->GetRenderContext(), commandPool()); command_pool.ResetCommandPool(); return true; };
        _graphActions.push_back(action);
    }
    
    template<typename PassDesc>
    void MakePass(PassDesc* parameters) {
        assert(_renderGraph && "Unable to create a new pass, render graph is null");

        if(_renderGraph) {
            RenderPassAction action;
            action.func = [this, parameters]() -> bool { typename PassDesc::PassType(_renderGraph, parameters, _graphIdentifier).Execute(); return true; };
            _graphActions.push_back(action);
        }
    };
    
private:
    RenderGraph* _renderGraph;
    std::vector<GenericInstanceWrapper<IGraphAction>> _graphActions;

    // The graph identifier is to help identify resources that should be created with render passes and we want to keep them cross render pass recreation
    std::string _graphIdentifier;
};
