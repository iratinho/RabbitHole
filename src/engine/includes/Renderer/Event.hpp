#pragma once
class Device;

// Follow metal implementation to allow command buffer to wait and encode signals

class Event {
public:
    virtual ~Event() = default;

    struct InitializationParams {
        Device* _device;
    };

public:
    static std::unique_ptr<Event> MakeEvent(const InitializationParams& params);

protected:
    virtual void Initialize() = 0;
    
protected:
    InitializationParams _params{};
};
