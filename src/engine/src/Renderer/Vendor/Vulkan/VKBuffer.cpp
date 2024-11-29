#include "Renderer/Vendor/Vulkan/VKBuffer.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureResource.hpp"

void VKBuffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    if(type == EBufferType::BT_HOST && usage == EBufferUsage::BU_Uniform) {
        type = (EBufferType)(type | EBufferType::BT_LOCAL);
        usage = (EBufferUsage)(usage | EBufferUsage::BU_Transfer);
    }
    
    Buffer::Initialize(type, usage, allocSize);

    VkBufferUsageFlags vkBufferUsage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
    
    if(_usage & EBufferUsage::BU_Geometry) {
        vkBufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if(_usage & EBufferUsage::BU_Uniform) {
        vkBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    
    if(_type & EBufferType::BT_HOST) {
        if(_usage & EBufferUsage::BU_Transfer) {
            vkBufferUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        if(vkBufferUsage == VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM) {
            assert(0);
            return;
        }

        std::tie(_stagingBuffer, _stagingBufferMemory) = MakeBuffer(vkBufferUsage, static_cast<VkMemoryPropertyFlagBits>(VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    }
    
    if(_type & EBufferType::BT_LOCAL) {
        if(_usage & EBufferUsage::BU_Transfer) {
            vkBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        if(vkBufferUsage == VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM) {
            assert(0);
            return;
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
    VkFunc::vkMapMemory(dynamic_cast<VKDevice *>(_device)->GetLogicalDeviceHandle(), _stagingBufferMemory, 0, _size, 0, &buffer);
    return buffer;
};

void VKBuffer::UnlockBuffer() {
    if(_stagingBufferMemory) {
        VkFunc::vkUnmapMemory(dynamic_cast<VKDevice *>(_device)->GetLogicalDeviceHandle(), _stagingBufferMemory);
    }
};

std::pair<VkBuffer, VkDeviceMemory> VKBuffer::MakeBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlagBits memoryFlags) const {
        VkBufferCreateInfo bufferCreateInfo {};
        bufferCreateInfo.flags = 0;
        bufferCreateInfo.size = _size;
        bufferCreateInfo.usage = usage;
        bufferCreateInfo.pNext = nullptr;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

        VkBuffer buffer;
        if (VkFunc::vkCreateBuffer(((VKDevice*)_device)->GetLogicalDeviceHandle(), &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS) {
            return {VK_NULL_HANDLE, VK_NULL_HANDLE};
        }
        
        VkMemoryRequirements memoryRequirements;
        VkFunc::vkGetBufferMemoryRequirements(((VKDevice*)_device)->GetLogicalDeviceHandle(), buffer, &memoryRequirements);

        unsigned int memoryTypeIndex = ((VKDevice*)_device)->FindMemoryTypeIndex(memoryFlags, memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo {};
        memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.pNext = nullptr;

        VkDeviceMemory memory;
        if (VkFunc::vkAllocateMemory(((VKDevice*)_device)->GetLogicalDeviceHandle(), &memoryAllocateInfo, nullptr, &memory) != VK_SUCCESS) {
            return {VK_NULL_HANDLE, VK_NULL_HANDLE};
        }
        
        // Associate our buffer with this memory
        VkFunc::vkBindBufferMemory(((VKDevice*)_device)->GetLogicalDeviceHandle(), buffer, memory, 0);

        return {buffer, memory};
}
