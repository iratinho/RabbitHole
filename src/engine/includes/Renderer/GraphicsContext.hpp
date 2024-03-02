#pragma once
#include "CommandEncoder.hpp"
#include "glm/glm.hpp"
#include <functional> //for std::hash
#include <string>
#include "Renderer/GraphBuilder.hpp"

struct GraphicsPipelineParams;
class GraphicsPipeline;
class RenderContext;
class RenderTarget;
class CommandQueue;
class RenderPass;

class GraphicsContext {
public:
    struct PassTarget {
        std::string _identifier;
        std::shared_ptr<RenderTarget> _renderTarget;
        Attachment _attachmentInfo;
        ImageLayout _targetLayout;
//        Ops _layoutOp; // This Ops is generic code, we dont need to provide oldLayout, we track it with all images
        glm::vec3 _clearColor;
    };
    
protected:
    struct PassContext {
        PassTarget _colorTarget;
        PassTarget _depthTarget;
        PassTarget _input;
        CommandCallback _callback;
        GraphicsPipeline* _pipeline;
    };
            
public:
    GraphicsContext();
    virtual ~GraphicsContext();
    
    /**
     * Creates a new graphic context backed by a render api
     *
     * @param renderContext
     */
    static std::shared_ptr<GraphicsContext> Create(std::shared_ptr<RenderContext> renderContext);
        
    [[nodiscard]] GraphBuilder& GetGraphBuilder() {
        return _graphBuilder;
    };
    
    void Flush();
    
    /**
     * @brief Initializes the graphics context resources
     *
     * @returns true if all graphics context resources were initialized
     */
    virtual bool Initialize() = 0;
    
    /**
     * @brief Get the Device object
     */
    virtual RenderContext* GetDevice() = 0;

    /**
     * @brief Construct a new Begin Frame object
     */
    virtual void BeginFrame() = 0;

    /**
     * @brief Construct a new End Frame object
     */
    virtual void EndFrame() = 0;
            
    /**
     * @brief Returns the graphics pipelines that we are currently using. Rememver that pipelines are shared between contexts
     *
     *  @return a  vector of pairs with string -> graphics pipeline where string is a pipeline identifier
     */
    virtual std::vector<std::pair<std::string, std::shared_ptr<GraphicsPipeline>>> GetPipelines() = 0;

    /**
     * @brief Gets the render target owned by the swapchain to draw color content
     *
     * @return std::shared_ptr<RenderTarget>
     */
    virtual std::shared_ptr<RenderTarget> GetSwapchainColorTarget() = 0;
    
    /**
     * @brief Gets the render target owned by the swapchain to draw depth content
     *
     * @return std::shared_ptr<RenderTarget>
     */
    virtual std::shared_ptr<RenderTarget> GetSwapchainDepthTarget() = 0;

    /**
     * @brief Executes all allocated pipelines for the graphics context
     */
    virtual void ExecutePipelines() = 0;

    /**
     * @brief Presents the results to screen
     */
    virtual void Present() = 0;
    
    virtual void Execute(RenderGraphNode &node) = 0;
            
protected:
    std::unique_ptr<CommandEncoder> _commandEncoder;

    //    std::unordered_map<uint32_t, std::unique_ptr<RenderPass>> _renderPass;
    std::shared_ptr<GraphicsPipeline> pipeline;
    GraphBuilder _graphBuilder;
    
private:
    
    
};
