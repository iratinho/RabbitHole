#pragma once
#include "GPUDefinitions.h"

class TextureResource;
class Device;
class TextureView;
class Range;

struct TextureParameters {
    unsigned int _width;
    unsigned int _height;
    unsigned int _samples;
    Format pixelFormat;
    TextureFlags flags = Tex_SAMPLED_OP;
};

class Texture2D {    
public:
    Texture2D() {}
    virtual ~Texture2D();
    
   /**
    * Creates a new texture 2D 
    */
    static std::shared_ptr<Texture2D> MakeFromPath(const char* path, Format pixelFormat);

    static std::shared_ptr<Texture2D> MakeFromExternalResource(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags = Tex_None, unsigned int levels = 0);
    
    static std::shared_ptr<Texture2D> MakeTexturePass(std::uint32_t width, std::uint32_t height, Format pixelFormat, TextureFlags flags = Tex_None, unsigned int levels = 0);
    /**
     * Initializes the texture 2d
     *
     * @param width
     * @param height
     * @param pixelFormat
     * @param levels used for max mip map level
     * @param flags that specify how this texture is going to be used, defaults for sampled image
     */
    virtual bool Initialize(Device* renderContext);
    
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
    
    /**
     * @brief Get the Texture Flags object
     * 
     * @return * Returns texture flags
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
     * @brief Set the Filter object
     * 
     * @param magFilter - filter for magnification
     * @param minFilter - filter for minification
     */
    void SetFilter(TextureFilter magFilter, TextureFilter minFilter);
    
    /**
     * @brief Set the Texture Layout object
     * 
     * @param imageLayout 
     */
    void SetTextureLayout(ImageLayout imageLayout) { _imageLayout = imageLayout; };
    

    /**
     * @brief Get the Current Layout object
     * 
     * @return ImageLayout 
     */
    [[nodiscard]] inline ImageLayout GetCurrentLayout() { return _imageLayout; };

    /**
     * @brief Gets the data size in bytes needed for this image
     *
     * @return Image data size in bytes
     */
    [[nodiscard]] size_t GetImageDataSize() const { return _dataSize; };
    
    /**
     * @brief Reloads texture from disk and store in CPU memory
     * 
     * @param bIsDeepReload - If true, will delete current data and reload from disk, if false it will only reload if data is empty
     */
    void Reload(bool bIsDeepReload = false);
    
    bool IsDirty();
    
protected:
    std::shared_ptr<TextureResource> _textureResource;
    Device* _device;
    std::shared_ptr<TextureView> _textureView;
    std::unordered_map<std::size_t, std::shared_ptr<TextureView>> _textureViews;
    std::uint32_t _height = 0;
    std::uint32_t _width = 0;
    TextureFlags _flags = Tex_SAMPLED_OP;
    Format _pixelFormat = Format::FORMAT_UNDEFINED;
    bool _hasExternalResource = false;
    ImageLayout _imageLayout = ImageLayout::LAYOUT_UNDEFINED;
    TextureFilter _magFilter = TextureFilter::NEAREST;
    TextureFilter _minFilter = TextureFilter::NEAREST;
    TextureWrapMode _wrapU = TextureWrapMode::REPEAT;
    TextureWrapMode _wrapV = TextureWrapMode::REPEAT;
    TextureWrapMode _wrapW = TextureWrapMode::REPEAT;
    unsigned char* _data  = nullptr; // CPU Pixel data from when we load an image from disk
    size_t _dataSize = 0;
    const char* _path = nullptr;
};
