#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VulkanLoader.hpp"
#include "Renderer/Vendor/Vulkan/VulkanTranslator.hpp"

VkDescriptorSet VKDescriptorManager::AcquireDescriptorSet(GraphicsContext* graphicsContext, const ShaderDataStream& dataStream) {
    std::size_t hash = 0;
    for (const ShaderDataBlock& block : dataStream._dataBlocks) {
        hash_combine(hash, block);
    }

    if(!_cache.Contains(hash)) {
        std::vector<VkDescriptorSetLayoutBinding> layoutsBindings;

        unsigned int binding = 0;
        for(const ShaderDataBlock& block : dataStream._dataBlocks) {
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding = binding;
            layoutBinding.descriptorCount = 1; // Only relevant if we had an array in the shader
            layoutBinding.descriptorType = TranslateShaderBlockUsage(block._usage);
            layoutBinding.stageFlags = TranslateShaderStage(block._stage); // TODO The shader stage should be moved to block level
            layoutBinding.pImmutableSamplers = nullptr;

            layoutsBindings.push_back(layoutBinding);
            binding++;
        }
            
        VkDescriptorSetLayoutCreateInfo layoutCreateInfo {};
        layoutCreateInfo.pBindings = layoutsBindings.data();
        layoutCreateInfo.bindingCount = (std::uint32_t)layoutsBindings.size();
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkResult result = VkFunc::vkCreateDescriptorSetLayout(((VKDevice*)graphicsContext->GetDevice())->GetLogicalDeviceHandle(), &layoutCreateInfo, nullptr, &layout);
        if(result != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        VKGraphicsContext* context = (VKGraphicsContext*)graphicsContext;
        if(!context) {
            VkFunc::vkDestroyDescriptorSetLayout(((VKDevice*)graphicsContext->GetDevice())->GetLogicalDeviceHandle(), layout, nullptr);
            return VK_NULL_HANDLE;
        }
        
        _cache.Put(hash, std::make_shared<VKDescriptorPool>(context->GetDescriptorPool(), layout));
    }
    
    auto pool = _cache.Get(hash);
    if(pool) {
        return pool->AcquireDescriptorSet(graphicsContext);
    }
    
    return VK_NULL_HANDLE;
}


void VKDescriptorManager::ResetPools() {
    for (auto [hash, pool] : _cache) {
        pool->Reset();
    }
}
