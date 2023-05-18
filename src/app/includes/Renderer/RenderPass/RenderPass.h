#pragma once
#include <unordered_map>
#include "Renderer/render_context.h"
#include "any"

class RenderGraph;

// A generic wrapper, this is very nice to create non heap allocated classes but still call them from base classes
template <typename Interface>
class GenericInstanceWrapper {
public:
    // TODO create enable if to create diff constructors for pointer and non pointer
    template <typename Implementation>
    GenericInstanceWrapper(Implementation&& concrete_render_pass)
        : storage(std::forward<Implementation>(concrete_render_pass))
        , getter([](std::any& storage) -> Interface& { return std::any_cast<Implementation&>(storage);} )
    {}

    bool Execute() {
        return (&this->getter(storage))->Execute();
    };
    
    Interface* operator->() { return &getter(storage); }
    
private:
    std::any storage;
    Interface& (*getter)(std::any&);
};

class IRenderPass {
public:
    virtual ~IRenderPass() = default;

    void Execute() {
        if(this->Initialize())
        {
            this->CreateFramebuffer();
            this->CreateCommandBuffer();
            this->RecordCommandBuffer();    
        }
    }

    std::vector<VkCommandBuffer> GetCommandBuffer() {
        return this->GetCommandBuffers();
    }

protected:
    virtual bool Initialize() = 0;
    virtual bool CreateFramebuffer() = 0;
    virtual bool CreateCommandBuffer() = 0;
    virtual bool RecordCommandBuffer() = 0;
    virtual std::vector<VkCommandBuffer> GetCommandBuffers() = 0;
};

class IFence {
public:
    virtual bool AllocateFence() = 0;
    virtual void ResetFence() = 0;
    virtual void WaitFence() = 0;
    virtual void* GetResource() = 0;
};

class IGraphAction {
public:
    virtual ~IGraphAction() = default;
    virtual bool Execute() = 0;
    std::any resource_;
};

class ISwapchain {
public:
    enum ESwapchainRenderTargetType
    {
        COLOR,
        DEPTH
    };
    
    virtual bool Initialize() = 0;
    virtual void Recreate() = 0;
    virtual bool RequestNewPresentableImage(uint32_t index) = 0;
    virtual void MarkSwapchainDirty() = 0;
    virtual bool IsSwapchainDirty() const = 0;
    virtual uint32_t GetNextPresentableImage() const = 0;
    virtual void* GetNativeHandle() const = 0;
};

