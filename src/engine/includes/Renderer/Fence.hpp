#pragma once

class RenderContext;

class Fence {
public:
    struct InitializationParams {
        std::shared_ptr<RenderContext> _renderContext;
    };

public:
    static std::unique_ptr<Fence> MakeFence(const InitializationParams& params);
    
public:
    virtual void Wait() = 0;

protected:
    virtual void Initialize() = 0;
    
protected:
    InitializationParams _params;
};
