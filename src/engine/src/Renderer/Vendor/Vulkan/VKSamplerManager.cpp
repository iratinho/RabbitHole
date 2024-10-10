#include "Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VulkanTranslator.hpp"
#include "Renderer/GraphicsContext.hpp"

VKSamplerManager::VKSamplerManager() {
}

VKSamplerManager::~VKSamplerManager() {
}

VkSampler VKSamplerManager::AcquireSampler(GraphicsContext *graphicsContext, const Sampler &sampler) {
    std:size_t hash = hash_value(sampler);
    if(_cache.Contains(hash)) {
        return _cache.Get(hash);
    }
    
    VKDevice* device = (VKDevice*)graphicsContext->GetDevice();
    if(!device) {
        assert(0);
        return VK_NULL_HANDLE;
    }
        
    VkSamplerCreateInfo createInfo {};
    createInfo.addressModeU = TranslateWrapMode(sampler._wrapU);
    createInfo.addressModeV = TranslateWrapMode(sampler._wrapV);
    createInfo.addressModeW = TranslateWrapMode(sampler._wrapW);
    createInfo.magFilter = TranslateFilter(sampler._magFilter);
    createInfo.minFilter = TranslateFilter(sampler._minFilter);
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    VkSampler vkSampler = VK_NULL_HANDLE;
    VkResult result = VkFunc::vkCreateSampler(device->GetLogicalDeviceHandle(), &createInfo, nullptr, &vkSampler);
    
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    _cache.Put(hash, vkSampler);
    return _cache.Get(hash);
}

