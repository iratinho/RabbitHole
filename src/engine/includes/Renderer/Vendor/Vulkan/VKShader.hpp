#pragma once
#include "Renderer/Shader.hpp"
#include "Renderer/GPUDefinitions.h"
#include "vulkan/vulkan.hpp"

class RenderContext;

class VKShader : public Shader {
public:
    using Shader::Shader;
    
    bool Compile() override;
    
    const VkPipelineShaderStageCreateInfo& GetShaderStageInfo() const {
        return _shaderStageInfo;
    }
            
private:
    VkPipelineShaderStageCreateInfo _shaderStageInfo;
        
    bool _bWasCompiled = false;
};
