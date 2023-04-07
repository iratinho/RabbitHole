#include "Renderer/render_context.h"
#include "Renderer/RenderTarget.h"

RenderTarget::RenderTarget(RenderContext* render_context, const TextureParams& params)
    : Texture(render_context, params)
    , image_view_(nullptr) {
}

RenderTarget::RenderTarget(Texture&& texture)
    : Texture(std::forward<Texture>(texture))
    , image_view_(nullptr)
{
}

RenderTarget::~RenderTarget()
{
    FreeResource();
}

bool RenderTarget::Initialize() {
    if(Texture::Initialize()) {
        return CreateResource();
    }
    
    return false;
}

bool RenderTarget::CreateResource() {
    const bool is_depth_renderTarget = params_.format == VK_FORMAT_D32_SFLOAT;
    
    VkImageSubresourceRange resources_ranges;
    resources_ranges.aspectMask = is_depth_renderTarget ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    resources_ranges.layerCount = 1;
    resources_ranges.levelCount = 1;
    resources_ranges.baseArrayLayer = 0;
    resources_ranges.baseMipLevel = 0;
    
    VkImageViewCreateInfo image_view_create_info {};
    image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
    image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
    image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
    image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
    image_view_create_info.flags = 0;
    image_view_create_info.format = static_cast<VkFormat>(params_.format);
    image_view_create_info.image = GetResource();
    image_view_create_info.pNext = nullptr;
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.subresourceRange = resources_ranges;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    const VkResult result = VkFunc::vkCreateImageView(render_context_->GetLogicalDeviceHandle(), &image_view_create_info, nullptr, &image_view_);
    return result == VK_SUCCESS;
}

void RenderTarget::FreeResource(bool only_view) {
    if(render_context_ && image_view_ != VK_NULL_HANDLE) {
        render_context_->DestroyImageView(image_view_);
        image_view_ = nullptr;
    }

    if(!only_view) {
        Texture::FreeResource();
    }
}
