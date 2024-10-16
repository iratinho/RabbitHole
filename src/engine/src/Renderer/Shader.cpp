#include "Renderer/Shader.hpp"
#include "Core/GenericFactory.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
using ResourceType = VKShader;
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
#ifdef USING_VULKAN_API
    return std::make_unique<VKShader>(device, pipeline, stage, params);
#endif

    return nullptr;
}
