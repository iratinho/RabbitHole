#pragma once
#include "GPUDefinitions.h"

class TextureResource;
class RenderContext;
class TextureView;
class Range;

struct TextureParameters {
    unsigned int _width;
    unsigned int _height;
    unsigned int _pixelFormat;
    unsigned int _samples;
    Format pixelFormat;
    TextureFlags flags = Tex_SAMPLED_OP;
};

class Texture2D {    
protected:
public:
    Texture2D(std::shared_ptr<RenderContext> renderContext);
    virtual ~Texture2D();
    
//    /**
//     * Creates a new texture 2D 
//     */
//    static std::shared_ptr<Texture2D> MakeTexture2D(std::shared_ptr<RenderContext> renderContext);
    
    /**
     * Initializes the texture 2d
     *
     * @param width
     * @param height
     * @param pixelFormat
     * @param levels used for max mip map level
     * @param flags that specify how this texture is going to be used, defaults for sampled image
     */
    virtual bool Initialize(std::uint32_t width, std::uint32_t height, Format pixelFormat, unsigned int levels, TextureFlags flags = Tex_SAMPLED_OP, bool bExternalResource = false);
    
    /**
     *  Creates a new texture view backed by the texture resource, this overload uses the
     *  current pixel and max image levels
     */
    virtual TextureView* MakeTextureView();
    
    /**
     * Creates a new texture view backed by the texture resource
     *
     *  @param format - pixel format for this view
     *  @param levels - mipmap level range that the view will have access
     */
    virtual TextureView* MakeTextureView(Format format, const Range &levels);
    
    /**
     * Creates texture 2d graphics resource
     */
    virtual void CreateResource(void* resourceHandle = nullptr);
    
    /**
     * Destroy texture 2d graphics resource
     */
    virtual void FreeResource();
        
    /**
     * Returns the texture resource backed by the graphics API
     */
    std::shared_ptr<TextureResource> GetResource();
    
    /**
     * Returns texture width
     */
    std::uint32_t GetWidth();
    
    /**
     * Returns texture height
     */
    std::uint32_t GetHeight();
    
    /*
     * Returns pixel format for the texture
     */
    Format GetPixelFormat();
    
    /*
     *  Returns texture flags
     */
    TextureFlags GetTextureFlags();
    
    /*
     * Sets the wrap modes for texture sampling.
     
     * @param wrapU - horizontal wrap mode
     * @param wrapV - vertical wrap mode
     * @param wrapW - depth wrap mode
     */
    void SetWrapModes(TextureWrapMode wrapU, TextureWrapMode wrapV, TextureWrapMode wrapW);
    
    /**
     * Returns a tupple with wrap modes in UVW order
     */
    const std::tuple<TextureWrapMode, TextureWrapMode, TextureWrapMode>& GetWrapModes();
    
    void SetTextureLayout(ImageLayout imageLayout) { _imageLayout = imageLayout; };
    
    [[nodiscard]] inline ImageLayout GetCurrentLayout() { return _imageLayout; };
    
protected:
    // UVW
    std::tuple<TextureWrapMode, TextureWrapMode, TextureWrapMode> _wrapModes;
    std::shared_ptr<TextureResource> _textureResource;
    std::shared_ptr<RenderContext> _renderContext;
    std::shared_ptr<TextureView> _textureView;
    std::unordered_map<std::size_t, std::shared_ptr<TextureView>> _textureViews;
    std::uint32_t _height;
    std::uint32_t _width;
    TextureFlags _flags;
    Format _pixelFormat;
    ImageLayout _imageLayout = ImageLayout::LAYOUT_UNDEFINED;
};
