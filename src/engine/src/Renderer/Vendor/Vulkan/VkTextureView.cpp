#include "Renderer/Vendor/Vulkan/VkTextureView.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Core/Utils.hpp"

void VkTextureView::CreateView(Format format, const Range& levels, TextureType textureType) {
    std::shared_ptr<VkTextureResource> textureResource = std::static_pointer_cast<VkTextureResource>(_textureResource);
    if(!textureResource) {
        return;
    }
    
    VkImageSubresourceRange resourcesRange;
    resourcesRange.aspectMask = TranslateFormat(format)  == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    resourcesRange.layerCount = 1;
    resourcesRange.levelCount = levels.count();
    resourcesRange.baseArrayLayer = 0;
    resourcesRange.baseMipLevel = 0;
    
    VkImageViewCreateInfo imageViewCreateInfo {};
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.format = TranslateFormat(format);
    imageViewCreateInfo.image =  textureResource->GetImage();
    imageViewCreateInfo.pNext = nullptr;
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.subresourceRange = resourcesRange;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    const VkResult result = VkFunc::vkCreateImageView(((VKDevice*)_device)->GetLogicalDeviceHandle(), &imageViewCreateInfo, nullptr, &_imageView);
}

void VkTextureView::FreeView() {
    VkFunc::vkDestroyImageView(((VKDevice*)_device)->GetLogicalDeviceHandle(), _imageView, VK_NULL_HANDLE);
}

