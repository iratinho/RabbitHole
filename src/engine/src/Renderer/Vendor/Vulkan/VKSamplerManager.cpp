#include "Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/render_context.hpp"

VKSamplerManager::VKSamplerManager() {
}

VKSamplerManager::~VKSamplerManager() {
}

VkSampler VKSamplerManager::AcquireSampler(GraphicsContext *graphicsContext, const Sampler &sampler) {
    std:size_t hash = hash_value(sampler);
    if(_cache.Contains(hash)) {
        return _cache.Get(hash);
    }
        
    VkSamplerCreateInfo createInfo {};
    createInfo.addressModeU = TranslateWrapMode(sampler._wrapU);
    createInfo.addressModeV = TranslateWrapMode(sampler._wrapV);
    createInfo.addressModeW = TranslateWrapMode(sampler._wrapW);
    createInfo.magFilter = TranslateFilter(sampler._magFilter);
    createInfo.minFilter = TranslateFilter(sampler._minFilter);
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    
    VkSampler vkSampler = VK_NULL_HANDLE;
    VkResult result = VkFunc::vkCreateSampler(graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &createInfo, nullptr, &vkSampler);
    
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    _cache.Put(hash, vkSampler);
    return _cache.Get(hash);
}

