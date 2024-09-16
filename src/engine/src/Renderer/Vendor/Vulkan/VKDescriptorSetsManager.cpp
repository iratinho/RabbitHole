#include "Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/VulkanTranslator.hpp"

VkDescriptorSet VKDescriptorManager::AcquireDescriptorSet(GraphicsContext* graphicsContext, const std::vector<ShaderInputResource> &inputResources) {
//    std::size_t hash = hash_value(inputResource);
    
    std::size_t hash = 0;
    for (auto& inputResource : inputResources) {
        hash_combine(hash, inputResource._binding);
    }
    
    if(!_cache.Contains(hash)) {
        std::vector<VkDescriptorSetLayoutBinding> layoutsBindings;
        
        for (auto& inputResource : inputResources) {
            VkDescriptorSetLayoutBinding layoutBinding;
            layoutBinding.binding = inputResource._binding._id;
            layoutBinding.descriptorCount = 1; // Only relevant if we had an array in the shader
            layoutBinding.descriptorType = TranslateShaderInputType(inputResource._binding._type);
            layoutBinding.stageFlags = TranslateShaderStage(inputResource._binding._shaderStage);
            layoutBinding.pImmutableSamplers = nullptr;
            
            layoutsBindings.push_back(layoutBinding);
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

void VKDescriptorManager::ResetPools() {
    for (auto [hash, pool] : _cache) {
        pool->Reset();
    }
}

// Think more about this there are multiple layers here...
// 1 - Layouts can be unique
// 2 - Allocated descriptor set must duplicated by swapchain image and each mesh should also have a unique descriptor set if the resources bound are different......
/*
    
    This manager already exists per frame so having it duplicated by swapchain is already done.
    Now we need to be able to create a descriptor set per mesh
 
    In the shader we could modify the ShaderInputParam to also include the resource that is relevant
    when its time to execute the material processor will bind the textures to the shader this will allows to have the connection we need
 
    In this manager we need to get an hash from that ShaderInputParam and create a new descriptor set if it does not match
 
    In the ProcessImp we can use the encoder to bind the shader resources? encoder->bindShaderResource(binding, resource)?
 
    Or should we directly work with the shaders?

 
    shader->AssignResource(matCapTexture, materialComponent._matCapTexture);
    shader->AssignResource(normalMap, materialComponent._normalMap);

    // bindShaderResources -> GraphicsContext -> VKDescriptorSetsCache -> Create -> VKUpdate (if new) -> Bind for draw
    // encoder->bindShaderResources(shader) // This bind shader resources should assert if not all the resources were bound
 
    the encoder goes to the graphics context to get this descriptor set manager and it will register the data here?
 
 */

//void VKDescriptorSetCache::BindDescriptorSet(Shader* shader) {
//    if(!shader) {
//        assert(0);
//        return;
//    }
//    
//    if(_descriptors.find(shader) == _descriptors.end()) {
//        return CreateDescriptor(shader);
//    }
//    
//    return _descriptors[shader];
//}
//
//VkDescriptorSet VKDescriptorSetCache::CreateDescriptor(Shader *shader) {
//    VKShader* vkShader = (VKShader*)shader;
//    
//    if(!vkShader) {
//        assert(0);
//        return VK_NULL_HANDLE;
//    }
//    
//    auto shaderResourceBindings = vkShader->GetShaderInputs();
//    
//    std::vector<VkDescriptorSetLayoutBinding> layoutsBindings;
//    
//    for (auto& resourceBinding : shaderResourceBindings) {
//        VkDescriptorSetLayoutBinding layoutBinding;
//        layoutBinding.binding = resourceBinding._id;
//        layoutBinding.descriptorCount = 1;
//        layoutBinding.descriptorType = TranslateShaderInputType(resourceBinding._type);
////        layoutBinding.pImmutableSamplers
//    }
//    
//    VkDescriptorSetLayoutCreateInfo layoutCreateInfo {};
//    layoutCreateInfo.pBindings = layoutsBindings.data();
//    layoutCreateInfo.bindingCount = layoutsBindings.size();
//    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    
//    // TODO we also should cache the descriptor set layout but where? Should we also use the same cache?
//    // Should we use the descriptor set layout as a key for multiple descriptor sets?
////    VkFunc::vkCreateDescriptorSetLayout();
//    
//    
//    return VK_NULL_HANDLE;
//
//    
////    auto& descriptorSetLayouts = vkShader->GetDescriptorSetLayouts();
////    
////    if(descriptorSetLayouts.size() > 0) {
////        VkDescriptorSetAllocateInfo allocInfo = {};
////        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
////        allocInfo.descriptorPool = ((VKGraphicsContext*)_graphicsContext)->GetDescriptorPool();
////        allocInfo.descriptorSetCount = descriptorSetLayouts.size();
////        allocInfo.pSetLayouts = descriptorSetLayouts.data();
////
////        VkDescriptorSet descriptorSet;
////        result = VkFunc::vkAllocateDescriptorSets(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &allocInfo, &descriptorSet);
////        if (result != VK_SUCCESS) {
////            assert(0);
////            return false;
////        }
////        
////        // Instead of shader we need resource hash + layout per descriptor
////        // Actually it makes sense to have a descriptor set per material since draw calls can share same material, so i need a hash made from material
////        // in a way i can identify the material uniqueness or by the node resources
////        _descriptors[shader] = descriptorSet;
////    }
////
////    return _descriptors[shader];
//}
