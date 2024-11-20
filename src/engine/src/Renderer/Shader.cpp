#include "Renderer/Shader.hpp"
#include "Core/GenericFactory.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
using ResourceType = VKShader;
#else
#include "Renderer/Vendor/WebGPU/WebGPUShader.hpp"
using ResourceType = WebGPUShader;
#endif

std::shared_ptr<Shader> Shader::MakeShader(GraphicsContext *_graphicsContext, const std::string &path, ShaderStage stage) {
    return nullptr;
//    return Core::Factory<StorageType, Storage>::GetOrCreate(path, _graphicsContext, path, stage);
}

std::shared_ptr<Shader> Shader::GetShader(GraphicsContext *_graphicsContext, const std::string &path, ShaderStage stage) {
    //    return Core::Factory<StorageType, Storage>::Get(path, _graphicsContext, path, stage);
    return nullptr;
}

std::unique_ptr<Shader> Shader::MakeShader(Device* device, GraphicsPipeline* pipeline, ShaderStage stage, const ShaderParams &params) {
    return std::make_unique<ResourceType>(device, pipeline, stage, params);
}

static std::unique_ptr<Shader> MakeShader(Device* device, GraphicsPipeline* pipeline, ShaderStage stage, ShaderSet* shaderSet) {
    return nullptr;
}
