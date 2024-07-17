#pragma once
#include "RenderCommandEncoder.hpp"
#include "glm/glm.hpp"
#include "Renderer/GraphBuilder.hpp"


/*
 We need a way to have a graph that can execute in multiple queues (graphics, compute, transfer), my ideia is that the
 graphics context can be a hub that facilitates the creation of those queues. The question now is how do we setup all of those queues?
 
    Option 1:
        - The constructor could have a create info structure and we would specify how and what queues would be needed
 
    Option 2:
        - During graph execution, we could identify what pass we are execution and based on that create a new queue if it dont exist yet
 
    I like the option 2, because it allow us to create them on deman when they are needed
 
 How would all of this work?
    
     The graph builder knows every node that needs to execute, i thinking about dividing the execution into execution blocks. Let's say
    that our graph contains 3 passes. Pass 1 and 3 are raster passes, while Pass 2 is a compute pass that needs to be executed before Pass 3.
    Since this passes needs to run in 2 queues (Raster and Compute)) we need to execute and submit commands from 1 and then from 2 and only then from 3.
 
 */

struct GraphicsPipelineParams;
class GraphicsPipeline;
class RenderContext;
class Texture2D;
class CommandQueue;
class RenderPass;

class GraphicsContext {
public:
    GraphicsContext();
    virtual ~GraphicsContext();
    
    /**
     * Creates a new graphic context backed by a render api
     *
     * @param renderContext
     */
    static std::shared_ptr<GraphicsContext> Create(std::shared_ptr<RenderContext> renderContext);
                
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
    virtual std::shared_ptr<Texture2D> GetSwapChainColorTexture() = 0;
    
    /**
     * @brief Gets the render target owned by the swapchain to draw depth content
     *
     * @return std::shared_ptr<RenderTarget>
     */
    virtual std::shared_ptr<Texture2D> GetSwapChainDepthTexture() = 0;

    /**
     * @brief Executes all allocated pipelines for the graphics context
     */
    virtual void ExecutePipelines() = 0;

    /**
     * @brief Presents the results to screen
     */
    virtual void Present() = 0;
    
    virtual void Execute(RenderGraphNode node) = 0;
            
protected:
    RenderCommandEncoder* _commandEncoder;

    //    std::unordered_map<uint32_t, std::unique_ptr<RenderPass>> _renderPass;
    std::shared_ptr<GraphicsPipeline> pipeline;    
    
};
