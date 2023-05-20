#include "Renderer/render_context.hpp"
#include "Renderer/RenderTarget.hpp"

RenderTarget::RenderTarget(RenderContext* render_context, const RenderTargetParams& params)
    : _params(params)
    , _renderContext(render_context) {
    _texture = std::make_shared<Texture>(_renderContext, _params._textureParams);
}

RenderTarget::~RenderTarget() {
    RenderTarget::FreeResource();
}

bool RenderTarget::Initialize()
{
    if(_texture && _texture->Initialize()) {
        return CreateView();
    }
    
    return false;
}

void RenderTarget::FreeResource()
{
    if(_renderContext && IsValidResource()) {
        _renderContext->DestroyImageView(_imageView);
        _imageView = VK_NULL_HANDLE;

        _texture->FreeResource();
    }
}

void RenderTarget::SetTextureResource(void* resource) {
    if(_texture) {
        _texture->SetResource(resource);
    }
}

unsigned RenderTarget::GetWidth() const {
    return _params._textureParams._width;
}

unsigned RenderTarget::GetHeight() const {
    return _params._textureParams._height;
}

void* RenderTarget::GetView() const {
    return _imageView;
}

const void* RenderTarget::GetTextureResource() const {
    if(_texture) {
        return _texture->GetResource();
    }

    return nullptr;
}

bool RenderTarget::IsValidResource() const {
    return _imageView != VK_NULL_HANDLE && _texture && _texture->IsValidResource();
}

std::shared_ptr<ITextureInterface> RenderTarget::GetTexture() const {
    return _texture;
}

bool RenderTarget::CreateView() {
    if (!_texture) {
        return false;
    }

    if (!_texture->GetResource()) {
        std::cerr << "[Error]: Trying to create render target resource with invalid texture resource." << std::endl;
        return false;
    }
    
    VkImageSubresourceRange resourcesRange;
    resourcesRange.aspectMask = _params._textureParams.format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    resourcesRange.layerCount = 1;
    resourcesRange.levelCount = 1;
    resourcesRange.baseArrayLayer = 0;
    resourcesRange.baseMipLevel = 0;
    
    VkImageViewCreateInfo imageViewCreateInfo {};
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.format = _params._textureParams.format;
    imageViewCreateInfo.image =  static_cast<const VkImage>(_texture->GetResource());
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.subresourceRange = resourcesRange;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    const VkResult result = VkFunc::vkCreateImageView(_renderContext->GetLogicalDeviceHandle(), &imageViewCreateInfo, nullptr, &_imageView);

    if(result != VK_SUCCESS) {
        std::cerr << "[Error]: Failed to create image view for render target." << std::endl;
    }

    return result == VK_SUCCESS;
}
