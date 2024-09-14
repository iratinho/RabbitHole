#pragma once
class RenderContext;

// Follow metal implementation to allow command buffer to wait and encode signals

class Event {
public:
    struct InitializationParams {
        RenderContext* _renderContext;
    };

public:
    static std::unique_ptr<Event> MakeEvent(const InitializationParams& params);

protected:
    virtual void Initialize() = 0;
    
protected:
    InitializationParams _params;
};
