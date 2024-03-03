#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/ShaderCompiler.hpp"

bool VKShader::Compile() {
    if(_bWasCompiled)
        return true;
    
    std::vector<char> shaderCode = std::move(ShaderCompiler::Get().CompileStatic(_path.c_str(), _stage));

    VkShaderModuleCreateInfo moduleCreateInfo {};
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = shaderCode.size() * sizeof(char);
    moduleCreateInfo.pCode = reinterpret_cast<const unsigned int*>(shaderCode.data());
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    const VkResult result = VkFunc::vkCreateShaderModule(_renderContext->GetLogicalDeviceHandle(), &moduleCreateInfo, nullptr, &shaderModule);

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

    for (auto& pushConstant : _constants) {
        if(pushConstant._shaderStage == ShaderStage::STAGE_VERTEX) {
            if(!_vertexConstantRange.has_value()) {
                _vertexConstantRange.emplace();
            }
        
            _vertexConstantRange->offset = 0;
            _vertexConstantRange->stageFlags = TranslateShaderStage(pushConstant._shaderStage);
            _vertexConstantRange->size += pushConstant._size;
        }
        else {
            if(!_fragmentConstantRange.has_value()) {
                _fragmentConstantRange.emplace();
            }

            _fragmentConstantRange->offset = _vertexConstantRange.has_value() ? _vertexConstantRange->size : 0;
            _fragmentConstantRange->stageFlags = TranslateShaderStage(pushConstant._shaderStage);
            _fragmentConstantRange->size += pushConstant._size;
        }
    }

    _bWasCompiled = true;
    
    return true;
}
