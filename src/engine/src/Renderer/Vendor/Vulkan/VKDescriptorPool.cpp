#include "Renderer/Vendor/Vulkan/VKDescriptorPool.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/VulkanLoader.hpp"

VkDescriptorSet VKDescriptorPool::AcquireDescriptorSet(GraphicsContext* graphicsContext) {
    if(_pool.HasFreeObjects()) {
        return _pool.GetObject();
    }
    
    AllocateDescriptors(graphicsContext);
    return _pool.GetObject();
}

void VKDescriptorPool::AllocateDescriptors(GraphicsContext* graphicsContext) {
    if(!graphicsContext || !_descriptorPool || _parentLayout == VK_NULL_HANDLE) {
        assert(0);
        return;
    }
        
    constexpr std::size_t size = 1;
    std::array<VkDescriptorSetLayout, size> layouts;
    layouts.fill(_parentLayout);
    
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = layouts.size();
    allocInfo.pSetLayouts = layouts.data();
    
    std::array<VkDescriptorSet, size> descriptorSets;
    VkResult result = VkFunc::vkAllocateDescriptorSets(((VKDevice*)graphicsContext->GetDevice())->GetLogicalDeviceHandle(), &allocInfo, descriptorSets.data());
    if (result != VK_SUCCESS) {
        assert(0);
        return;
    }
    
    _pool.Insert(descriptorSets.begin(), descriptorSets.end());
}

void VKDescriptorPool::Reset() {
    _pool.Reset();
}
