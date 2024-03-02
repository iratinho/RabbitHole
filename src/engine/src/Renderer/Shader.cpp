#include "Renderer/Shader.hpp"
#include "Core/GenericFactory.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
using StorageType = VKShader;
using Storage = Core::StorageCache<VKShader, std::string>;
#endif

std::shared_ptr<Shader> Shader::MakeShader(RenderContext *renderContext, const std::string &path, ShaderStage stage) {
    return Core::Factory<StorageType, Storage>::Create(path, renderContext, path, stage);
}

