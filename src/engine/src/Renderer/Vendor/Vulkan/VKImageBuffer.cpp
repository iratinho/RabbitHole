#include "Renderer/Vendor/Vulkan/VKImageBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Texture2D.hpp"

VKImageBuffer::VKImageBuffer(Device* device, std::weak_ptr<TextureResource> resource)
    : _resource(resource)
{
    _device = device;
}

void VKImageBuffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    Buffer::Initialize(type, usage, allocSize);
    
    // CPU buffer
    if(type & EBufferType::BT_HOST) {
        VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        std::tie(_stagingBuffer, _stagingBufferMemory) = MakeBuffer(flags, (VkMemoryPropertyFlagBits) (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));  
    }

    if(type & EBufferType::BT_LOCAL) {
        VkImage image = VK_NULL_HANDLE;

        if(!_resource.expired()) {
            auto vkResource = std::static_pointer_cast<VkTextureResource>(_resource.lock());
            if(vkResource) {
                image = vkResource->GetImage();
            }
        }

        VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        _localBufferMemory = MakeImageBuffer(flags, (VkMemoryPropertyFlagBits) VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image);
    }
};

VkDeviceMemory VKImageBuffer::MakeImageBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryFlags, VkImage image) {
    if(image == VK_NULL_HANDLE) {
        assert(0);
        return VK_NULL_HANDLE;
    }
        
    VkMemoryRequirements requirements;
    VkFunc::vkGetImageMemoryRequirements(((VKDevice*)_device)->GetLogicalDeviceHandle(), image, &requirements);
        
    unsigned int memoryTypeIndex = ((VKDevice*)_device)->FindMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requirements);
    
    VkMemoryAllocateInfo allocInfo {};
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = requirements.size;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;

    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkFunc::vkAllocateMemory(((VKDevice*)_device)->GetLogicalDeviceHandle(), &allocInfo, nullptr, &memory);
    VkFunc::vkBindImageMemory(((VKDevice*)_device)->GetLogicalDeviceHandle(), image, memory, 0);

    return memory;
}

