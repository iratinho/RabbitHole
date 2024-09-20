#pragma once

#include "Renderer/GPUDefinitions.h"

class Range;
class Device;
class Texture2D;
class TextureResource;

class TextureView {
public:
    TextureView(Device* device, std::shared_ptr<TextureResource> textureResource);
    virtual ~TextureView() = default;
    
    static std::unique_ptr<TextureView> MakeTextureView(Device* device, std::shared_ptr<TextureResource> textureResource);
    
    /*
     * Creates the texture view
     *
     * @param format - texture pixel format
     * @param levels - mip map levels that this view as access
     * @param textureType
     */
    virtual void CreateView(Format format, const Range& levels, TextureType textureType) = 0;
    
    /*
     * Frees the texture view
     */
    virtual void FreeView() = 0;
    
protected:
    Device* _device = nullptr;
    std::shared_ptr<TextureResource> _textureResource = nullptr;
    
private:
    void Cleanup() {
        FreeView();
    };
};
