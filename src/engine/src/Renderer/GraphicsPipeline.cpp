#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Core/GenericFactory.hpp"
#include "Core/Cache/Cache.hpp"
#include "Renderer/Shader.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
using ResourceType = VKGraphicsPipeline;
#else
#include "Renderer/Vendor/WebGPU/WebGPUPipeline.hpp"
using ResourceType = WebGPUGraphicsPipeline;
#endif

static Core::Cache<std::size_t, ResourceType> _cache;

GraphicsPipeline::GraphicsPipeline(const GraphicsPipelineParams& params)
    : _params(params) {
}

GraphicsPipeline::~GraphicsPipeline() {};

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineParams& params) {
    std::size_t hash = hash_value(params._rasterization, params._fsParams._shaderPath, params._vsParams._shaderPath);
    
    _cache.Put(hash, std::make_shared<ResourceType>(params));
    return _cache.Get(hash);
}

Shader* GraphicsPipeline::GetVertexShader() {
    return _vertexShader.get();
}

Shader* GraphicsPipeline::GetFragmentShader() {
    return _fragmentShader.get();
}

bool GraphicsPipeline::CompileShaders() {
    _vertexShader = Shader::MakeShader(_params._device, this, ShaderStage::STAGE_VERTEX, _params._vsParams);
    _fragmentShader = Shader::MakeShader(_params._device, this, ShaderStage::STAGE_FRAGMENT, _params._fsParams);
    
    if(!_vertexShader || !_fragmentShader) {
        return false;
    }
    
    bool bCompliedVertexShader = _vertexShader->Compile();
    bool bCompileFragmentShader = _fragmentShader->Compile();
    
    if(!bCompliedVertexShader || !bCompileFragmentShader) {
        _vertexShader.reset();
        _fragmentShader.reset();
        return false;
    }
    
    return true;
}
