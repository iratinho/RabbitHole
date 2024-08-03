#pragma once
#include "Renderer/Buffer.hpp"

class RenderContext;
class Texture2D;

class TextureResource {
public:
    TextureResource(RenderContext* renderContext, Texture2D* texture, bool bIsExternalResource);
    virtual ~TextureResource() {};
    
    static std::unique_ptr<TextureResource> MakeResource(RenderContext* renderContext, Texture2D* texture, bool bIsExternalResource = false);
    
    /**
     * Creates a new texture resource handled by the graphics API
     */
    virtual void CreateResource() = 0;
        
    /**
     *  Sets a new resource handle managed by external entity,
     */
    virtual void SetExternalResource(void* handle) = 0;
    
    /**
     * Fress the texture resource handled by the graphics API
     */
    virtual void FreeResource() = 0;

    /**
     * Returns resource validitiy
     */
    virtual bool HasValidResource() = 0;
    
    /**
     * @brief Returns a pointer to the image buffer
     */
    virtual void* Lock() = 0;
    
    /**
    * @brief 
     */
    virtual void Unlock() = 0;
    
    bool IsDirty();
    
public:
    std::shared_ptr<Buffer> GetBuffer() {
        return _buffer;
    }
        
protected:
    RenderContext* _renderContext = nullptr;
    Texture2D* _texture = nullptr;
    bool _bIsExternalResource = false;
    
    // Buffer used to hold GPU / CPU data
    std::shared_ptr<Buffer> _buffer;
};
