#include "Renderer/Vendor/Vulkan/VkTexture2D.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"

VkSampler VkTexture2D::MakeSampler() {
    VkSamplerCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	info.pNext = nullptr;
	info.magFilter = TranslateFilter(_magFilter);
	info.minFilter = TranslateFilter(_minFilter);
	info.addressModeU = TranslateWrapMode(_wrapU);
	info.addressModeV = TranslateWrapMode(_wrapV);
    info.addressModeW = TranslateWrapMode(_wrapW);

    // TODO we need to cache samplers, so we don't create a new one every time
    VkSampler sampler;
    VkFunc::vkCreateSampler(_graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &info, nullptr, &sampler);
    
    return sampler;
}