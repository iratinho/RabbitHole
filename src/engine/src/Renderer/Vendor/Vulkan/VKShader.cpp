#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VulkanTranslator.hpp"
#include "Renderer/Vendor/Vulkan/ShaderCompiler.hpp"

bool VKShader::Compile() {
    if(_bWasCompiled)
        return true;
    
    _path = _params._shaderPath;
    
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
    VkResult result = VkFunc::vkCreateShaderModule(((VKDevice*)_device)->GetLogicalDeviceHandle(), &moduleCreateInfo, nullptr, &shaderModule);
    
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

    _bWasCompiled = true;
    
    return true;
}
