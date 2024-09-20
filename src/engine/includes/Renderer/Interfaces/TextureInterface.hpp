#pragma once
#include "Renderer/GPUDefinitions.h"

typedef struct TextureParams {
    Format format = Format::FORMAT_UNDEFINED;
    TextureFlags flags = Tex_None;
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
    [[nodiscard]] virtual Format GetFormat() const = 0;
    [[nodiscard]] virtual void* GetView() const = 0;
};
