#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"
#include "Renderer/VulkanLoader.hpp"

void VKBuffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    
    Buffer::Initialize(type, usage, allocSize);        

    VkBufferUsageFlags vkBufferUsage;
    
    if(_usage & EBufferUsage::BU_Geometry) {
        vkBufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    
    if(_type & EBufferType::BT_HOST) {
        if(_usage & EBufferUsage::BU_Transfer) {
            vkBufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        std::tie(_stagingBuffer, _stagingBufferMemory) = MakeBuffer(vkBufferUsage, (VkMemoryPropertyFlagBits) (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    }
    
    if(_type & EBufferType::BT_LOCAL) {
        if(_usage & EBufferUsage::BU_Transfer) {
            vkBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        std::tie(_localBuffer, _localBufferMemory) = MakeBuffer(vkBufferUsage, (VkMemoryPropertyFlagBits) VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
    
    // TODO unmapp and destroy buffers on destructor
//    VkFunc::vkMapMemory(_renderContext->GetLogicalDeviceHandle(), _stagingBufferMemory, 0, _size, 0, &_cpuData);
}

void* VKBuffer::LockBuffer() {
    if(_stagingBufferMemory == VK_NULL_HANDLE) {
        return nullptr;
    }
    
    void* buffer = nullptr;
    VkFunc::vkMapMemory(_renderContext->GetLogicalDeviceHandle(), _stagingBufferMemory, 0, _size, 0, &buffer);
    return buffer;
};

void VKBuffer::UnlockBuffer() {
    if(_stagingBufferMemory) {
        VkFunc::vkUnmapMemory(_renderContext->GetLogicalDeviceHandle(), _stagingBufferMemory);
    }
};

std::pair<VkBuffer, VkDeviceMemory> VKBuffer::MakeBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryFlags) {
        VkBufferCreateInfo bufferCreateInfo {};
        bufferCreateInfo.flags = 0;
        bufferCreateInfo.size = _size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        VkBuffer buffer;
        if (VkFunc::vkCreateBuffer(_renderContext->GetLogicalDeviceHandle(), &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
            return {VK_NULL_HANDLE, VK_NULL_HANDLE};
        }
        
        VkMemoryRequirements memoryRequirements;
        VkFunc::vkGetBufferMemoryRequirements(_renderContext->GetLogicalDeviceHandle(), buffer, &memoryRequirements);

        unsigned int memoryTypeIndex = _renderContext->FindMemoryTypeIndex(memoryFlags, memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo {};
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;

        VkDeviceMemory memory;
        if (VkFunc::vkAllocateMemory(_renderContext->GetLogicalDeviceHandle(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS) {
            return {VK_NULL_HANDLE, VK_NULL_HANDLE};
        }
        
        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(_renderContext->GetLogicalDeviceHandle(), buffer, memory, 0);

        return {buffer, memory};
}
