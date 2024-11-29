#pragma once

class GraphicsContext;
class Device;
class RenderCommandEncoder;
class BlitCommandEncoder;
class Event;
class Fence;

class CommandBuffer {
public:
    struct InitializationParams {
        Device* _device;
    };
    
public:
    virtual ~CommandBuffer();
    
    static std::unique_ptr<CommandBuffer> MakeCommandBuffer(const CommandBuffer::InitializationParams& params);

    /*
     * Creates a new render command encoder managed by this command buffer
     */
    RenderCommandEncoder* MakeRenderCommandEncoder(GraphicsContext* graphicsContext, Device* device);
    
    void RemoveEncoder(RenderCommandEncoder* ptr);
    void RemoveEncoder(BlitCommandEncoder* ptr);

    /*
     * Creates a new blit command encoder managed by this command buffer
     */
    BlitCommandEncoder* MakeBlitCommandEncoder(GraphicsContext* graphicsContext, Device* device);
       
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
    
    /**
     * Presents the results of the command buffer commands to the presentation engine
    */
    virtual void Present() = 0;
            
protected:
    
    /**
     * Initializes the command buffer
     */
    virtual bool Initialize() = 0;
    
public:
    /**
     * Adds a wait event to the command buffer that will only execute the submited work when its signaled
     * (All encoded events are removed from the command buffer once we start recording again)
     */
    void EncodeWaitForEvent(std::shared_ptr<Event> event);
    
    /**
     * Adds a completion event to the command buffer that will be signaled when the GPU finishes executing its commands
     * (All encoded events are removed from the command buffer once we start recording again)
     */
    void EncodeSignalEvent(std::shared_ptr<Event> event);
    
    /**
     * Returns a list of events that will be signaled when the GPU finishes executing its commands
     */
    std::vector<std::shared_ptr<Event>> GetSignalEvents() const {
        return _signalEvents;
    };
    
protected:
    CommandBuffer::InitializationParams _params;
    
    std::vector<std::unique_ptr<RenderCommandEncoder>> _renderCommandEncoders;
    std::vector<std::unique_ptr<BlitCommandEncoder>> _blitCommandEncoders;

    std::vector<std::shared_ptr<Event>> _waitEvents;
    std::vector<std::shared_ptr<Event>> _signalEvents;
};
