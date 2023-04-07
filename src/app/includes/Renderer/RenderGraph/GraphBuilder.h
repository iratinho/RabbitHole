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
    const char* _source;
    ShaderType _type;
// public:
    // Type* _;
};

#define MAKE_VARADIC_LIST(...) __VA_OPT__

#define DECLARE_SHADER(type, sourc_, shaderType) struct type : public Shader<type> { type() { _source = sourc_; _type = shaderType; };
#define DECLARE_PARAMETER(type, name) public: type name;
#define END_SHADER_DECLARATION() };

#define DECLARE_DEFAULT_DESC_PARAMETERS() bool enabled_; int frameIndex;
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
        for (auto& graph_action : graph_actions_) {
            if(!graph_action.Execute()) {
                return false;
            }
        }

        return true;
    }

    void AcquireSwapchainImage(uint32_t index) {
        RequestNewPresentableImageAction action;
        action.func = [this, index]() -> bool { return render_graph_->GetRenderContext()->GetSwapchain()->RequestNewPresentableImage(index); };
        graph_actions_.push_back(action);
    };

    void AllocateCommandBuffer(uint32_t index) {
        AllocateCommandBufferAction action;
        action.func = [this, index]() -> bool { return render_graph_->GetCommandBufferManager()->AllocateCommandBuffer(index); };
        graph_actions_.push_back(action);
    }

    void EnableCommandBufferRecording(uint32_t index) {
        EnableCommandBufferRecordingAction action;
        action.func = [this, index]() -> bool { return render_graph_->GetRenderContext()->BeginCommandBuffer(render_graph_->GetCommandBufferManager()->GetCommandBuffer((int)index)); };
        graph_actions_.push_back(action);
    }

    void DisableCommandBufferRecording(uint32_t index) {
        DisableCommandBufferRecordingAction action;
        action.func = [this, index]() -> bool { return render_graph_->GetRenderContext()->EndCommandBuffer(render_graph_->GetCommandBufferManager()->GetCommandBuffer((int)index)); };
        graph_actions_.push_back(action);
    }

    void ReleaseCommandBuffer(uint32_t index) {
        ReleaseCommandBufferAction action;
        action.func = [this, index]() -> bool { return render_graph_->GetCommandBufferManager()->ReleaseCommandBuffer(index); };
        graph_actions_.push_back(action);
    }

    void WaitFence(Fence* fence) {
        WaitFenceAction action;
        action.func = [this, fence]() -> bool { fence->WaitFence(); return true; };
        graph_actions_.push_back(action);
    };

    void ResetFence(Fence* fence) {
        ResetFenceAction action;
        action.func = [this, fence]() -> bool { fence->ResetFence(); return true; };
        graph_actions_.push_back(action);
    };

    void ResetCommandPool(VkCommandPool commandPool) {
        ResetCommandPoolAction action;
        action.func = [this, commandPool]() -> bool { CommandPool command_pool (render_graph_->GetRenderContext(), commandPool); command_pool.ResetCommandPool(); return true; };
        graph_actions_.push_back(action);
    }

    void ResetCommandPoolLambda(std::function<VkCommandPool()> commandPool) {
        ResetCommandPoolAction action;
        action.func = [this, commandPool]() -> bool { CommandPool command_pool (render_graph_->GetRenderContext(), commandPool()); command_pool.ResetCommandPool(); return true; };
        graph_actions_.push_back(action);
    }
    
    template<typename PassDesc>
    void MakePass(PassDesc* parameters) {
        assert(render_graph_ && "Unable to create a new pass, render graph is null");

        if(render_graph_) {
            RenderPassAction action;
            action.func = [this, parameters]() -> bool { typename PassDesc::PassType(render_graph_, parameters, graph_identifier_).Execute(); return true; };
            graph_actions_.push_back(action);
        }
    };
    
private:
    RenderGraph* render_graph_;
    std::vector<GenericInstanceWrapper<IGraphAction>> graph_actions_;

    // The graph identifier is to help identify resources that should be created with render passes and we want to keep them cross render pass recreation
    std::string graph_identifier_;
};
