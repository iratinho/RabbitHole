#pragma once
#include "vulkan/vulkan.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/GPUDefinitions.h"
#include "Core/Cache/Cache.hpp"

class VKSamplerManager {
public:
    VKSamplerManager();
    ~VKSamplerManager();
    
    VkSampler AcquireSampler(GraphicsContext* graphicsContext, const Sampler& sampler);
private:
    Core::Cache<std::size_t, VkSampler> _cache;
};

