#pragma once
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKDescriptorPool.hpp"
#include "Core/Cache/Cache.hpp"

class VKDescriptorManager {
public:
    VkDescriptorSet AcquireDescriptorSet(GraphicsContext* graphicsContext, const std::vector<ShaderInputResource>& inputResource);
    
    // TODO
    void ResetPools() {};
private:
    Core::Cache<std::size_t, VKDescriptorPool> _cache;
};
