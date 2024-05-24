#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/ShaderCompiler.hpp"

bool VKShader::Compile() {
    if(_bWasCompiled)
        return true;
    
    std::vector<unsigned int> shaderCode = std::move(ShaderCompiler::Get().Compile(_path.c_str(), _stage));
    
    if(shaderCode.size() == 0)
        return false;

    VkShaderModuleCreateInfo moduleCreateInfo {};
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = shaderCode.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = reinterpret_cast<const unsigned int*>(shaderCode.data());
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = VkFunc::vkCreateShaderModule(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &moduleCreateInfo, nullptr, &shaderModule);

    if (result != VK_SUCCESS) {
        return false;
    }
    
    _shaderStageInfo.flags = 0;
    _shaderStageInfo.module = shaderModule;
    _shaderStageInfo.stage = TranslateShaderStage(_stage);
    _shaderStageInfo.pName = "main";
    _shaderStageInfo.pNext = nullptr;
    _shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _shaderStageInfo.pSpecializationInfo = nullptr;
    
    if(_stage == ShaderStage::STAGE_VERTEX) {        
        for(auto& [key, value] : _inputAttr) {
            VkVertexInputBindingDescription bindingDescriptor {};
            bindingDescriptor.binding = key._binding;
            bindingDescriptor.stride = key._stride;
            bindingDescriptor.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            inputBindings.push_back(bindingDescriptor);
            
            int i = 0;
            for(auto& location : value) {
                VkVertexInputAttributeDescription attributeDescriptor {};
                attributeDescriptor.binding = key._binding;
                attributeDescriptor.location = i;
                attributeDescriptor.offset = location._offset;
                attributeDescriptor.format = TranslateFormat(location._format);
                inputAttributes.push_back(attributeDescriptor);
                
                i++;
            }
        }
        
        _shaderVertexInputInfo.emplace();
        _shaderVertexInputInfo->flags = 0;
        _shaderVertexInputInfo->pNext = nullptr;
        _shaderVertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        _shaderVertexInputInfo->pVertexAttributeDescriptions = inputAttributes.data();
        _shaderVertexInputInfo->vertexAttributeDescriptionCount = static_cast<unsigned int>(inputAttributes.size());
        _shaderVertexInputInfo->pVertexBindingDescriptions = inputBindings.data();
        _shaderVertexInputInfo->vertexBindingDescriptionCount = static_cast<unsigned int>(inputBindings.size());
    }
    
    if(_stage == ShaderStage::STAGE_VERTEX && _constants.size() > 0) {
        if(!_vertexConstantRange.has_value()) {
            _vertexConstantRange.emplace();
        }
        
        _vertexConstantRange->offset = 0;
        _vertexConstantRange->stageFlags = TranslateShaderStage(ShaderStage::STAGE_VERTEX);
        _vertexConstantRange->size = GetPushConstantsSize();
    }
    
    if(_stage == ShaderStage::STAGE_FRAGMENT && _constants.size() > 0) {
        // Temporary solution, somehow we need to have vertex shader context, this assumes that shader names are the same..
        std::filesystem::path shaderPath(_path);
        shaderPath.replace_extension(".vert");

        std::shared_ptr<VKShader> vertexShader = std::dynamic_pointer_cast<VKShader>(Shader::GetShader(_graphicsContext, shaderPath.string(), ShaderStage::STAGE_FRAGMENT));
        
        if (!vertexShader) {
            assert(0 && "Trying to create push constants for fragment shader without having a vertex shader");
            return false;
        }

        if(!_fragmentConstantRange.has_value()) {
            _fragmentConstantRange.emplace();
        }
        
        _fragmentConstantRange->offset = vertexShader->GetPushConstantsSize();
        _fragmentConstantRange->stageFlags = TranslateShaderStage(ShaderStage::STAGE_FRAGMENT);
        _fragmentConstantRange->size = GetPushConstantsSize();
    }

    // TODO this needs a fix, push constants between shader stages need to know the offset based on the last ones
    // Example: VS shadeer has push constantes so when creating the FS shader the push constants need to take in
    // consideration the previous push constants size and used it in the offset_pa
    // I guess we can create an access function in the shader cache to get the shader here
    // for (auto& pushConstant : _constants) {
    //     if(pushConstant._shaderStage == ShaderStage::STAGE_VERTEX) {
    //         if(!_vertexConstantRange.has_value()) {
    //             _vertexConstantRange.emplace();
    //         }
        
    //         _vertexConstantRange->offset = 0;
    //         _vertexConstantRange->stageFlags = TranslateShaderStage(pushConstant._shaderStage);
    //         _vertexConstantRange->size += pushConstant._size;
    //     }
    //     else {
    //         if(!_fragmentConstantRange.has_value()) {
    //             _fragmentConstantRange.emplace();
    //         }

    //         _fragmentConstantRange->offset = _vertexConstantRange.has_value() ? _vertexConstantRange->size : 0;
    //         _fragmentConstantRange->stageFlags = TranslateShaderStage(pushConstant._shaderStage);
    //         _fragmentConstantRange->size += pushConstant._size;
    //     }
    // }
    

    /*
    *   Information regarding the descriptor set layouts and reflection in the shader
    *   In shader code we have the following:
    *   layout(set=0, binding=0)
    * 
    *   With vulkan the set=value is defined by the VkDescriptorSetLayout that we give to our VkPipelineLayout (VkPipelineLayoutCreateInfo::setLayoutCount and VkPipelineLayoutCreateInfo::pSetLayouts)
    *   The binding=value is defined by the VkDescriptorSetLayoutBinding that we give to our VkDescriptorSetLayout (VkDescriptorSetLayoutCreateInfo::pBindings)
    * 
    *   When creating a new shader input using the DeclareShaderInput, the ShaderInputParam struct will have an id field, this will define the input group in the shader (set)
    */

    
    // Sort _shaderInputs by id
    std::sort(_shaderInputs.begin(), _shaderInputs.end(), [](const ShaderInputParam& a, const ShaderInputParam& b) {
        return a._id < b._id;
    });

    std::map<int, std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;

    for (int i = 0; i < _shaderInputs.size(); i++)
    {    
        ShaderInputParam input = _shaderInputs[i];

        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding {};
        descriptorSetLayoutBinding.binding = i;
        descriptorSetLayoutBinding.descriptorCount = 1;
        descriptorSetLayoutBinding.stageFlags = TranslateShaderStage(input._shaderStage);

        if(input._type == ShaderInputType::UNIFORM_BUFFER) {
            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }

        if(input._type == ShaderInputType::TEXTURE) {
            descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        }

        layoutBindings[input._id].push_back(descriptorSetLayoutBinding);
    }

    // In vulkan the order for set layouts will dictate the set index in the shader, so we use the input.id as key in the map to keep the order
    for (auto [key, value] : layoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
        descriptorSetLayoutInfo.bindingCount = value.size();
        descriptorSetLayoutInfo.flags = 0;
        descriptorSetLayoutInfo.pBindings = value.data();
        descriptorSetLayoutInfo.pNext = nullptr;
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        result = VkFunc::vkCreateDescriptorSetLayout(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
        if (result != VK_SUCCESS) {
            assert(0);
            return false;
        }

        _descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    if(_descriptorSetLayouts.size() > 0) {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = ((VKGraphicsContext*)_graphicsContext)->GetDescriptorPool();
        allocInfo.descriptorSetCount = _descriptorSetLayouts.size();
        allocInfo.pSetLayouts = _descriptorSetLayouts.data();

        VkDescriptorSet descriptorSet;
        result = VkFunc::vkAllocateDescriptorSets(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &allocInfo, &descriptorSet);
        if (result != VK_SUCCESS) {
            assert(0);
            return false;
        }
    }

    _bWasCompiled = true;
    
    return true;
}

size_t VKShader::GetPushConstantsSize() const {
    size_t size = 0;
    for(auto& pushConstant : _constants) {
        size += pushConstant._size;
    }
    
    return size;
}
