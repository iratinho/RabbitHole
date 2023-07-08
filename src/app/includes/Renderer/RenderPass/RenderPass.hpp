#pragma once
#include "Renderer/render_context.hpp"
#include "Core/GenericInstanceWrapper.hpp"
#include <any>

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_BYTE_CODE_DIR) "/" STR(name)

class RenderGraph;

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
    
protected:
    std::any _actionData;

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

