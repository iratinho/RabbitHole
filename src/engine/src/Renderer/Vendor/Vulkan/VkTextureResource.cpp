#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"

void VkTextureResource::CreateResource() {
    if(_texture) {
        VkExtent3D extent {};
        extent.width = _texture->GetWidth();
        extent.height = _texture->GetHeight();
        extent.depth = 1;
        
        VkImageCreateInfo createInfo {};
        createInfo.extent = extent;
        createInfo.flags = 0;
        createInfo.format = TranslateFormat(_texture->GetPixelFormat());
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.usage = TranslateTextureUsageFlags(_texture->GetTextureFlags());
        createInfo.arrayLayers = 1;
        createInfo.imageType = VK_IMAGE_TYPE_2D;
        createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createInfo.mipLevels = 1;
        createInfo.pNext = nullptr;
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        
        VkResult result = VkFunc::vkCreateImage(_renderContext->GetLogicalDeviceHandle(), &createInfo, nullptr, &_image);
        
        VkMemoryRequirements requirements;
        VkFunc::vkGetImageMemoryRequirements(_renderContext->GetLogicalDeviceHandle(), _image, &requirements);
        
        // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT will not hold true for images that we need to inject pixel data
        int memoryTypeIndex = _renderContext->FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requirements);
        
        VkMemoryAllocateInfo allocInfo {};
        allocInfo.memoryTypeIndex = memoryTypeIndex;
        allocInfo.allocationSize = requirements.size;
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        
        VkFunc::vkAllocateMemory(_renderContext->GetLogicalDeviceHandle(), &allocInfo, nullptr, &_memory);
        VkFunc::vkBindImageMemory(_renderContext->GetLogicalDeviceHandle(), _image, _memory, 0);
    }
}

void VkTextureResource::SetExternalResource(void* handle) {
    _image = reinterpret_cast<VkImage>(handle);
}

void VkTextureResource::FreeResource() {
    VkFunc::vkDestroyImage(_renderContext->GetLogicalDeviceHandle(), _image, nullptr);
}

VkImage VkTextureResource::GetImage() {
    return _image;
}
