#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/render_context.hpp"

VkDescriptorSet VKDescriptorManager::AcquireDescriptorSet(GraphicsContext* graphicsContext, const std::vector<ShaderInputResource> &inputResource) {
    std::size_t hash = hash_value(inputResource);
    
    if(!_cache.Contains(hash)) {
        std::vector<VkDescriptorSetLayoutBinding> layoutsBindings;
        
        for (auto& inputResource : inputResource) {
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding = inputResource._binding._id;
            layoutBinding.descriptorCount = 1; // Only relevant if we had an array in the shader
            layoutBinding.descriptorType = TranslateShaderInputType(inputResource._binding._type);
            
            layoutsBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo {};
        layoutCreateInfo.pBindings = layoutsBindings.data();
        layoutCreateInfo.bindingCount = (std::uint32_t)layoutsBindings.size();
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkResult result = VkFunc::vkCreateDescriptorSetLayout(graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &layoutCreateInfo, nullptr, &layout);
        if(result != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        VKGraphicsContext* context = (VKGraphicsContext*)graphicsContext;
        if(!context) {
            VkFunc::vkDestroyDescriptorSetLayout(graphicsContext->GetDevice()->GetLogicalDeviceHandle(), layout, nullptr);
            return;
        }
        
        _cache.Put(hash, std::make_shared<VKDescriptorPool>(context->GetDescriptorPool(), layout));
    }
    
    auto pool = _cache.Get(hash);
    if(pool) {
        return pool->AcquireDescriptorSet(graphicsContext);
    }
    
    return VK_NULL_HANDLE;
}

