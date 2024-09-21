#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/Buffer.hpp"

void VkTextureResource::CreateResource() {
    if(_texture) {
        // We do not need to create a buffer because this resource is externally managed
        if(_bIsExternalResource) {
            return;
        }
        
        // Sampled images need to inject pixel data, so we need to create a staging and local buffer
        if(_texture->GetTextureFlags() & TextureFlags::Tex_SAMPLED_OP) {
            _buffer = Buffer::Create(_device, _texture->GetResource());
            const size_t allocSize = _texture->GetImageDataSize();
            if(allocSize == 0) {
                assert(0 && "To create a sampled texture resource we first need to call Reload to compute the required width and height so that we can get the image alloc size");
                return;
            }
            _buffer->Initialize((EBufferType)(BT_LOCAL | BT_HOST), BU_Texture, allocSize);
        } else {
            _buffer = Buffer::Create(_device, _texture->GetResource());
            _buffer->Initialize((EBufferType)BT_LOCAL, BU_Texture, 0);
        }
    }
}

void VkTextureResource::SetExternalResource(void* handle) {
    // Cast the handle to VkImage
    VkImage image = reinterpret_cast<VkImage>(handle);
        
    // Assign the image to _image
    _image = image;
}

void VkTextureResource::FreeResource() {
    _buffer.reset();
    if(_device && _image && !_bIsExternalResource) {
        VkFunc::vkDestroyImage(((VKDevice*)_device)->GetLogicalDeviceHandle(), _image, nullptr);
    }
}

bool VkTextureResource::HasValidResource() {
    return _image != VK_NULL_HANDLE;
}

void* VkTextureResource::Lock() {
    return _buffer->LockBuffer();
};

void VkTextureResource::Unlock() {
    return _buffer->UnlockBuffer();
};

VkImage VkTextureResource::GetImage() {
    if(_image == VK_NULL_HANDLE) {
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
        
        VkResult result = VkFunc::vkCreateImage(((VKDevice*)_device)->GetLogicalDeviceHandle(), &createInfo, nullptr, &_image);
        if(result != VK_SUCCESS) {
            return nullptr;
        }
    }
    
    return _image;
}
