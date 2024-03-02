#pragma once
#include "Renderer/Interfaces/TextureInterface.hpp"

class RenderContext;

class Texture : public ITextureInterface {
public:
    Texture() = default;
    Texture(RenderContext* render_context, TextureParams params);
    ~Texture();
    
    /** ----- ITextureInterface  ----- **/
    bool Initialize() override;
    void SetResource(void* resource) override;
    void FreeResource() override;
    void WritePixelData(void* data, size_t size) override;
    [[nodiscard]] inline unsigned int GetWidth() const override;
    [[nodiscard]] inline unsigned GetHeight() const override;
    [[nodiscard]] void* GetResource() const override;
    [[nodiscard]] bool IsValidResource() const override;
    [[nodiscard]] Format GetFormat() const override;

    ImageLayout GetImageLayout();
    void SetImageLayout(ImageLayout layout);
    void SetDescriptorSet();
    
    bool CreateView();
    [[nodiscard]] void* GetView() const override;

    
protected:
    unsigned int _textureWidth {};
    unsigned int _textureHeight {};
    
    VkImageUsageFlags TranslateTextureUsageFlags(const TextureFlags& usageFlags);
    TextureParams _params;
    RenderContext* _renderContext {};

private:
    VkImage _image {};
    VkDeviceMemory _deviceMemory {};
    VkMemoryRequirements _memoryRequirements {};
    ImageLayout _imageLayout;
    VkDescriptorSet _descriptorSet;
    VkImageView _imageView {};
};
