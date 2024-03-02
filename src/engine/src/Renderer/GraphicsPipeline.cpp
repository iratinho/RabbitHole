#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Core/GenericFactory.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VkGraphicsPipeline.hpp"
using StorageType = VKGraphicsPipeline;
using Storage = Core::StorageCache<StorageType, int>;
#endif

std::shared_ptr<GraphicsPipeline> GraphicsPipeline::Create(const GraphicsPipelineParams& params) {
    return Core::Factory<StorageType, Storage>::Create(params._id, params);
}
