#pragma once
#include <unordered_map>
#include "render_context.h"
#include "any"

class RenderGraph;

class IRenderPass {
public:
    virtual bool CreateCachedPSO() = 0;
    virtual bool CreateFramebuffer() = 0;
    virtual bool CreateCommandBuffer() = 0;
    virtual bool RecordCommandBuffer() = 0;
    virtual std::vector<VkCommandBuffer> GetCommandBuffers() = 0;
};

template <typename Interface>
class RenderPass {
public:
    template <typename Implementation>
    RenderPass(Implementation&& concrete_render_pass)
        : storage(std::forward<Implementation>(concrete_render_pass))
        , getter([](std::any& storage) -> Interface& { return std::any_cast<Implementation&>(storage);} )
    {}

    void Execute() {
        this->getter(storage).CreateCachedPSO();
        this->getter(storage).CreateFramebuffer();
        this->getter(storage).CreateCommandBuffer();
        this->getter(storage).RecordCommandBuffer();
    };

    std::vector<VkCommandBuffer> GetCommandBuffers()
    {
        return this->getter(storage).GetCommandBuffers();
    }
    
    Interface* operator->() { return &getter(storage); }
    
private:
    std::any storage;
    Interface& (*getter)(std::any&);
};
