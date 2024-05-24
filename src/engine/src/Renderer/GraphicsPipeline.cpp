#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Core/GenericFactory.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkGraphicsPipeline.hpp"
using StorageType = VKGraphicsPipeline;
using Storage = Core::StorageCache<StorageType, int>;
#endif

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineParams& params) {
    std::size_t hash = hash_value(params._rasterization, params._id, params._vertexShader->GetHash(), params._fragmentShader->GetHash());
    return Core::Factory<StorageType, Storage>::GetOrCreate(hash, params);
}
