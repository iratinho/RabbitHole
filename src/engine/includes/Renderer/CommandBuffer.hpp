#pragma once

class RenderContext;
class CommandEncoder;
class Event;
class Fence;

class CommandBuffer {
public:
    struct InitializationParams {
        std::shared_ptr<RenderContext> _renderContext;
    };
    
public:
    static std::unique_ptr<CommandBuffer> MakeCommandBuffer(const CommandBuffer::InitializationParams& params);

    /*
     * Creates a new command encoder managed by this command buffer
     */
    CommandEncoder* MakeRenderCommandEncoder(std::shared_ptr<RenderContext> renderContext);
       
public:
    /**
     * Signals the command buffer to start recording rendering commands
     */
    virtual void BeginRecording() = 0;
    
    /**
     * Signals the command buffer to stop recording rendering commands
     */
    virtual void EndRecording() = 0;
    
    /*
     * Sunmits the command buffer to the GPU
     *
     * @param fence - optional fence that will be trigered when the
     *  GPU finishes the commands execution
     */
    virtual void Submit(std::shared_ptr<Fence> fence = nullptr);
            
protected:
    
    /**
     * Initializes the command buffer
     */
    virtual bool Initialize() = 0;
    
public:
    /**
     * Adds a wait event to the command buffer that will only execute the submited work when its signaled
     * (All encoded events are removed from the command buffer once its submited)
     */
    void EncodeWaitForEvent(std::shared_ptr<Event> event);
    
    /**
     * Adds a completion event to the command buffer that will be signaled when the GPU finishes executing its commands
     * (All encoded events are removed from the command buffer once its submited)
     */
    void EncodeSignalEvent(std::shared_ptr<Event> event);
    
protected:
    CommandBuffer::InitializationParams _params;
    
    std::vector<std::unique_ptr<CommandEncoder>> _renderCommandEncoders;
    std::vector<std::shared_ptr<Event>> _waitEvents;
    std::vector<std::shared_ptr<Event>> _signalEvents;

};


// Create command buffer per render pass and them submit all at ocne, this will better align with Metal API
