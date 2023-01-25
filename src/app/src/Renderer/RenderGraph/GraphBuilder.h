#pragma once
#include <functional>
#include <glm.hpp>
#include <unordered_map>
#include "RenderGraph.h"
#include "RenderPass.h"

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

#define DECLARE_SHADER(type, sourc_, shaderType) struct type : public Shader<type> { type() { _source = sourc_; _type = shaderType; };
#define DECLARE_PARAMETER(type, name) public: type name;
#define END_SHADER_DECLARATION() };

#define DECLARE_DEFAULT_DESC_PARAMETERS() bool enabled_;
#define DECLARE_PASS_DESC(name, vs_shader_type, ps_shader_type, pass) class name { private: vs_shader_type vs_shader_; ps_shader_type ps_shader; friend class pass; public: using PassType = pass; DECLARE_DEFAULT_DESC_PARAMETERS() 
#define DECLARE_PASS_PARAMETER(type, name) type name;
#define END_PASS_DESC_DECLARATION() };

class GraphBuilder {
public:
    GraphBuilder() = default;
    GraphBuilder(RenderGraph* render_graph, std::string identifier);

    void Execute() {
        if(render_graph_->GetCachedCommandPool(graph_identifier_) == nullptr) {
            render_graph_->RegisterCommandPool(graph_identifier_, AllocateCommandPool());
        }
        
        for (auto& pass : passes_) {
            pass.Execute();
        }
    }

    template<typename PassDesc>
    void MakePass(PassDesc parameters) {
        assert(render_graph_ && "Unable to create a new pass, render graph is null");

        if(render_graph_) {
            passes_.push_back(typename PassDesc::PassType(render_graph_, parameters, graph_identifier_));
        }
    };

    std::vector<VkCommandBuffer> GetCommandBuffers()
    {
        std::vector<VkCommandBuffer> framebuffers;
        for (auto& pass : passes_) {
            auto buffers = pass.GetCommandBuffers();
            framebuffers.insert(std::end(framebuffers), std::begin(buffers), std::end(buffers)); 
        }

        return framebuffers;
    }

    VkCommandPool GetCommandPool() { return render_graph_->GetCachedCommandPool(graph_identifier_); }

private:
    VkCommandPool AllocateCommandPool();
    
    RenderGraph* render_graph_;
    std::vector<RenderPass<IRenderPass>> passes_;

    // The graph identifier is to help identify resources that should be created with render passes and we want to keep them cross render pass recreation
    std::string graph_identifier_;
    
    PersistentRenderTargets persistent_render_targets_;
};
