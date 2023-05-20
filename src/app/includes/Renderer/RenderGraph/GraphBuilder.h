#pragma once
#include "Renderer/CommandPool.h"
#include <glm.hpp>

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
    
    bool Execute();

    void AllocateCommandPool(CommandPool* commandPool);

    void AcquirePresentableSurface(uint32_t index);
    
    void AllocateCommandBuffer(CommandPool* commandPool);

    void EnableCommandBufferRecording(CommandPool* commandPool);

    void DisableCommandBufferRecording(CommandPool* commandPool);

    void ReleaseCommandBuffer(CommandPool* commandPool);

    void AllocateFence(const std::shared_ptr<Fence>& fence);
    
    void WaitFence(Fence* fence);
    
    void ResetFence(Fence* fence);

    void ResetCommandPool(CommandPool* commandPool);

    void SubmitCommands(CommandPool* commandPool, const SubmitCommandParams& submitCommandParams);
    
    void Present(const std::shared_ptr<Surface>& surface, const SurfacePresentParams& presentParams);

    void AllocateSurface(const std::shared_ptr<Surface>& surface, SurfaceCreateParams& params);
    
    void CopyGeometryData(std::shared_ptr<Buffer> buffer, const MeshNode* meshNode);

    void UploadBufferData(std::shared_ptr<Buffer> buffer, CommandBuffer* commandBuffer);
    
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
