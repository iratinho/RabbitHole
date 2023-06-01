#pragma once
#include "Renderer/render_context.hpp"

typedef enum TextureUsageFlags
{
    Tex_None = 0,
    Tex_COLOR_ATTACHMENT = 1 << 0,
    Tex_DEPTH_ATTACHMENT = 1 << 1,
    Tex_TRANSFER_DEST_OP = 1 << 2,
    Tex_SAMPLED_OP = 1 << 3,
    Tex_PRESENTATION = 1 << 4
} TextureUsageFlags;

typedef struct TextureParams {
    VkFormat format = VK_FORMAT_UNDEFINED;
    TextureUsageFlags flags = Tex_None;
    unsigned _width = 0;
    unsigned _height = 0;
    unsigned _sampleCount = 0;
    bool _hasSwapchainUsage:1 = false;
} TextureParams;

class ITextureInterface {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    virtual ~ITextureInterface() = default;

    virtual bool Initialize() = 0;
    virtual void SetResource(void* resource) = 0;
    virtual void FreeResource() = 0;
    virtual void WritePixelData(void* data, size_t size) = 0; // consider having functions to store the data in cpu side, right now its always on gpu
    
    /** Getters **/
    [[nodiscard]] virtual unsigned int GetWidth() const = 0;
    [[nodiscard]] virtual unsigned int GetHeight() const = 0;
    [[nodiscard]] virtual void* GetResource() const = 0;
    [[nodiscard]] virtual bool IsValidResource() const = 0;
};