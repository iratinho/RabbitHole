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
    
    const std::optional<VkPipelineVertexInputStateCreateInfo>& GetVertexInputInfo() const {
        return _shaderVertexInputInfo;
    }
    
    const std::optional<VkPushConstantRange>& GetVertexConstantRange() const {
        return _vertexConstantRange;
    }
    
    const std::optional<VkPushConstantRange>& GetFragmentConstantRange() const {
        return _fragmentConstantRange;
    }

    const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const {
        return _descriptorSetLayouts;
    }
    
    VkDescriptorSetLayout GetDescriptorSetLayout() const {
        return _descriptorSetLayout;
    }

    
private:
    [[nodiscard]] size_t GetPushConstantsSize() const;
    
    [[nodiscard]] size_t GetPushConstantsOffset() const;

private:
    VkPipelineShaderStageCreateInfo _shaderStageInfo;
    std::optional<VkPipelineVertexInputStateCreateInfo> _shaderVertexInputInfo;
    std::optional<VkPushConstantRange> _vertexConstantRange;
    std::optional<VkPushConstantRange> _fragmentConstantRange;
    
    std::vector<VkVertexInputBindingDescription> inputBindings;
    std::vector<VkVertexInputAttributeDescription> inputAttributes;
    std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;
    
    VkDescriptorSetLayout _descriptorSetLayout;

    
    bool _bWasCompiled = false;
};
