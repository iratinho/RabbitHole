#pragma once
#include "TextureInterface.hpp"

class ITextureInterface;

typedef enum RenderTargetUsageFlags
{
    Rt_None,
    Rt_Swapchain,
    Rt_UI
} RenderTargetUsageFlags;

typedef struct RenderTargetParams {
    TextureParams _textureParams;
    RenderTargetUsageFlags _usageFlags;
} RenderTargetParams;

class IRenderTargetInterface {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    virtual ~IRenderTargetInterface() = default;

    virtual bool Initialize() = 0;
    virtual void FreeResource() = 0;
    virtual void SetTextureResource(void* resource) = 0;
    
    /** Getters **/
    [[nodiscard]] virtual unsigned int GetWidth() const = 0;
    [[nodiscard]] virtual unsigned int GetHeight() const = 0;
    [[nodiscard]] virtual void* GetView() const = 0;
    [[nodiscard]] virtual const void* GetTextureResource() const = 0;
    [[nodiscard]] virtual bool IsValidResource() const = 0;
    [[nodiscard]] virtual std::shared_ptr<ITextureInterface> GetTexture() const = 0;
};
