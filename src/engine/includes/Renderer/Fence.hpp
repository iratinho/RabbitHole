#pragma once

class Device;

class Fence {
public:
    virtual ~Fence() = default;

    struct InitializationParams {
        Device* _device;
    };

public:
    static std::unique_ptr<Fence> MakeFence(const InitializationParams& params);
    
public:
    virtual void Wait() = 0;

protected:
    virtual void Initialize() = 0;
    
protected:
    InitializationParams _params{};
};
