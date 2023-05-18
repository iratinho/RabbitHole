#pragma once
#include "Renderer/CommandPool.h"
#include <functional>
#include <glm.hpp>
#include <utility>

#include "Actions/CommandPoolAction.h"
#include "Actions/FenceAction.h"
#include "Actions/RenderPassAction.h"
#include "Actions/SurfaceAction.h"
#include "Actions/SwapchainAction.h"
#include "Actions/BufferAction.h"
#include "Core/Components/SceneComponent.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/RenderGraph/RenderGraph.h"
#include "Renderer/RenderPass/RenderPass.h"
#include "Renderer/Fence.h"
#include "Renderer/Swapchain.h"

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

#define DECLARE_SHADER(type, sourc_, shaderType) struct type : public Shader<type> { type() { _source = sourc_; _type = shaderType; };
#define DECLARE_PARAMETER(type, name) public: type name;
#define END_SHADER_DECLARATION() };

#define DECLARE_DEFAULT_DESC_PARAMETERS() bool enabled_; int frameIndex; unsigned int previousPassIndex; unsigned int nextPassIndex; class CommandPool* _commandPool = nullptr; const std::vector<MeshNode>* meshNodes;
#define DECLARE_PASS_DESC(name, vs_shader_type, ps_shader_type, pass) class name { private: vs_shader_type vs_shader_; ps_shader_type ps_shader; friend class pass; public: using PassType = pass; DECLARE_DEFAULT_DESC_PARAMETERS() 
#define DECLARE_PASS_PARAMETER(type, name) type name;
#define END_PASS_DESC_DECLARATION() };

class GraphBuilder {
public:
    GraphBuilder() = default;
    GraphBuilder(RenderGraph* render_graph, const std::string& identifier);

    bool Execute() {
        for (auto& graph_action : _graphActions) {
            if(!graph_action.Execute()) {
                continue;
            }
        }

        return true;
    }

    void AllocateCommandPool(CommandPool* commandPool)
    {
        CommandPoolAction action;
        action._commandPoolAction = CPA_Allocate;
        action._renderGraph = _renderGraph;
        action._commandPool = commandPool;
        _graphActions.emplace_back(action);
    }

    void AcquirePresentableSurface(uint32_t index) {
        SwapchainAction action;
        action._swapchainAction = SCA_RequestImage;
        action._swapchain = _renderGraph->GetRenderContext()->GetSwapchain();
        action._index = index;
        _graphActions.emplace_back(action);
    }

    void AllocateCommandBuffer(CommandPool* commandPool) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_AllocateCommandBuffer;
        action._commandPool = commandPool;
        action._renderGraph = _renderGraph;
        _graphActions.emplace_back(action);
    }

    void EnableCommandBufferRecording(CommandPool* commandPool) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_EnableCommandBufferRecording;
        action._commandPool = commandPool;
        action._renderGraph = _renderGraph;
        _graphActions.emplace_back(action);
    }

    void DisableCommandBufferRecording(CommandPool* commandPool) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_DisableCommandBufferRecording;
        action._commandPool = commandPool;
        action._renderGraph = _renderGraph;
        _graphActions.emplace_back(action);
    }

    void ReleaseCommandBuffer(CommandPool* commandPool) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_ReleaseCommandBuffer;
        action._commandPool = commandPool;
        action._renderGraph = _renderGraph;
        _graphActions.emplace_back(action);
    }

    void AllocateFence(const std::shared_ptr<Fence>& fence) {
        FenceAction action;
        action._fenceAction = FA_Allocate;
        action._fence = fence.get();
        _graphActions.emplace_back(action);
    }
    
    void WaitFence(Fence* fence) {
        FenceAction action;
        action._fenceAction = FA_Wait;
        action._fence = fence;
        _graphActions.emplace_back(action);
    }

    void ResetFence(Fence* fence) {
        FenceAction action;
        action._fenceAction = FA_Reset;
        action._fence = fence;
        _graphActions.emplace_back(action);
    }

    void ResetCommandPool(CommandPool* commandPool) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_Reset;
        action._commandPool = commandPool;
        action._renderGraph = _renderGraph;
        _graphActions.emplace_back(action);
    }

    void SubmitCommands(CommandPool* commandPool, const SubmitCommandParams& submitCommandParams) {
        CommandPoolAction action;
        action._commandPoolAction = CPA_Submit;
        action._commandPool = commandPool;
        action._submitCommandsParams = submitCommandParams;
        _graphActions.emplace_back(action);
    }

    void Present(const std::shared_ptr<Surface>& surface, const SurfacePresentParams& presentParams) {
        SurfaceAction action;
        action._surfaceAction = SA_Present;
        action._surfacePresentParams = presentParams;
        action._surface = surface;
        _graphActions.emplace_back(action);
    }

    void AllocateSurface(const std::shared_ptr<Surface>& surface, SurfaceCreateParams& params) {
        SurfaceAction action;
        action._surfaceAction = SA_Allocate;
        action._surfaceCreateParams = params;
        action._surface = surface;
        _graphActions.emplace_back(action);
    }

    void CopyGeometryData(std::shared_ptr<Buffer> buffer, const MeshNode* meshNode) {
        BufferAction action;
        action._bufferAction = EBufferAction::BA_StageGeometryData;
        action._renderContext = _renderGraph->GetRenderContext();
        action._buffer = std::move(buffer);
        action._meshNode = meshNode;
        
        _graphActions.emplace_back(action);
    }

    void UploadBufferData(std::shared_ptr<Buffer> buffer, CommandBuffer* commandBuffer) {
        BufferAction action;
        action._bufferAction = EBufferAction::BA_TransferToGPU;
        action._renderContext = _renderGraph->GetRenderContext();
        action._buffer = std::move(buffer);
        action._commandBuffer = commandBuffer;
        
        _graphActions.emplace_back(action);
    }
    
    template<typename PassDesc>
    void MakePass(PassDesc* parameters) {
        assert(_renderGraph && "Unable to create a new pass, render graph is null");

        if(_renderGraph) {
            RenderPassAction<PassDesc> action;
            action._graphIdentifier = _graphIdentifier;
            action._renderGraph = _renderGraph;
            action._passDescription = parameters;
            _graphActions.push_back(action);
        }
    }
    
private:
    RenderGraph* _renderGraph {};
    std::vector<GenericInstanceWrapper<IGraphAction>> _graphActions;

    // The graph identifier is to help identify resources that should be created with render passes and we want to keep them cross render pass recreation
    std::string _graphIdentifier;
};
