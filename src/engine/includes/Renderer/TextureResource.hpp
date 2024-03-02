#pragma once

class RenderContext;
class Texture2D;

class TextureResource {
public:
    TextureResource(RenderContext* renderContext, Texture2D* texture);
    virtual ~TextureResource() {
        Cleanup();
    };
    
    static std::unique_ptr<TextureResource> MakeResource(RenderContext* renderContext, Texture2D* texture);
    
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
    
private:
    void Cleanup() {
        FreeResource();
    }
    
protected:
    RenderContext* _renderContext = nullptr;
    Texture2D* _texture = nullptr;
};
