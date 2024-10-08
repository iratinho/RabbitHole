#pragma once
#include "vulkan/vulkan.hpp"
#include "GPUDefinitions.h"

inline VkShaderStageFlagBits TranslateShaderStage(ShaderStage shaderStage) {
    switch (shaderStage) {
        case STAGE_VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case STAGE_FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: break;
    }

    assert(0);
    return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

inline VkFormat TranslateFormat(Format format) {
    switch (format) {
        case Format::FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case Format::FORMAT_R8G8B8_SNORM:
            return VK_FORMAT_R8G8B8_SNORM;
        case Format::FORMAT_B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case Format::FORMAT_R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case Format::FORMAT_R32G32_UNORM:
            return VK_FORMAT_R32G32_UINT;
        case Format::END_COLOR_FORMATS:
            break;
        case Format::FORMAT_D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case Format::FORMAT_R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case Format::FORMAT_R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case Format::END_DEPTH_FORMATS:
            break;
        case Format::FORMAT_UNDEFINED:
            return VK_FORMAT_UNDEFINED;
        default: break;
    }

    assert(0 && "Invalid vulkan format translation");
    return VK_FORMAT_MAX_ENUM;
};

inline VkImageUsageFlags TranslateTextureUsageFlags(const TextureFlags &usageFlags) {
    VkImageUsageFlags flags = 0;

    if (usageFlags & Tex_COLOR_ATTACHMENT) {
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if (usageFlags & Tex_DEPTH_ATTACHMENT) {
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if (usageFlags & Tex_TRANSFER_DEST_OP) {
        flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (usageFlags & Tex_SAMPLED_OP) {
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    return flags;
}

inline VkAttachmentLoadOp TranslateLoadOP(LoadOp loadOp) {
    switch (loadOp) {
        case LoadOp::OP_CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOp::OP_LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOp::OP_DONT_CARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
}

inline VkAttachmentStoreOp TranslateStoreOP(StoreOp storeOp) {
    switch (storeOp) {
        case StoreOp::OP_STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case StoreOp::OP_DONT_CARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
}

inline VkCompareOp TranslateCompareOP(CompareOperation compareOp) {
    switch (compareOp) {
        case CompareOperation::NEVER:
            return VK_COMPARE_OP_NEVER;
        case CompareOperation::LESS:
            return VK_COMPARE_OP_LESS;
        case CompareOperation::EQUAL:
            return VK_COMPARE_OP_EQUAL;
        case CompareOperation::LESS_OR_EQUAL:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOperation::GREATER:
            return VK_COMPARE_OP_GREATER;
        case CompareOperation::NOT_EQUAL:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOperation::GREATER_OR_EQUAL:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOperation::ALWAYS:
            return VK_COMPARE_OP_ALWAYS;
        default: break;
    }

    assert(0);
    return VK_COMPARE_OP_MAX_ENUM;
}

inline VkCullModeFlagBits TranslateCullMode(TriangleCullMode cullMode) {
    switch (cullMode) {
        case CullMode::CULL_MODE_NONE:
            return VK_CULL_MODE_NONE;
        case CullMode::CULL_MODE_FRONT:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::CULL_MODE_BACK:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::CULL_MODE_ALL:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }

    return VkCullModeFlagBits::VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

inline VkFrontFace TranslateWindingOrder(TriangleWindingOrder windingOrder) {
    switch (windingOrder) {
        case WindingOrder::CLOCK_WISE:
            return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
        case WindingOrder::COUNTER_CLOCK_WISE:
            return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    return VkFrontFace::VK_FRONT_FACE_MAX_ENUM;
}

inline VkBlendFactor TranslateBlendFactor(BlendFactor blendFactor) {
    switch (blendFactor) {
        case BlendFactor::BLEND_FACTOR_ZERO:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::BLEND_FACTOR_ONE:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::BLEND_FACTOR_SRC_COLOR:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::BLEND_FACTOR_SRC_ALPHA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    }

    return VkBlendFactor::VK_BLEND_FACTOR_MAX_ENUM;
}

inline VkBlendOp TranslateBlendOperation(BlendOperation blendOperation) {
    switch (blendOperation) {
        case BlendOperation::BLEND_OP_ADD:
            return VK_BLEND_OP_ADD;
    }

    return VkBlendOp::VK_BLEND_OP_MAX_ENUM;
}

inline VkImageLayout TranslateImageLayout(ImageLayout layout) {
    switch (layout) {
        case ImageLayout::LAYOUT_UNDEFINED:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case ImageLayout::LAYOUT_COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        case ImageLayout::LAYOUT_PRESENT:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        case ImageLayout::LAYOUT_TRANSFER_DST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageLayout::LAYOUT_SHADER_READ:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        default: {
            break;
        }
    }

    assert(0);
    return VK_IMAGE_LAYOUT_UNDEFINED;
}

inline VkFilter TranslateFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::NEAREST:
            return VK_FILTER_NEAREST;
        case TextureFilter::LINEAR:
            return VK_FILTER_LINEAR;
    }

    return VK_FILTER_MAX_ENUM;
}

inline VkSamplerAddressMode TranslateWrapMode(TextureWrapMode wrapMode) {
    switch (wrapMode) {
        case TextureWrapMode::REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TextureWrapMode::MIRRORED_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TextureWrapMode::CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TextureWrapMode::CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case TextureWrapMode::MIRROR_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }

    return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

inline std::pair<VkPipelineStageFlags, VkPipelineStageFlags> GetPipelineStageFlagsFromLayout(
    VkImageLayout oldLayout, VkImageLayout newLayout) {
    if (oldLayout == newLayout) {
        assert(0);
    }

    int error = 0;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    // PRESENT -> COLOR_ATTACHMENT / DEPTH_ATTACHMENT
    if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && (
            newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
        srcStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                       ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                       : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dstStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                       ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                       : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        error++;
    }

    // UNDEFINED -> COLOR_ATTACHMENT / DEPTH_ATTACHMENT
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && (
            newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || newLayout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)) {
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                       ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                       : VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

        error++;
    }

    // COLOR_ATTACHMENT -> PRESENT
    if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        error++;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

        error++;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        error++;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        error++;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        error++;
    }

    assert(error);

    return {srcStage, dstStage};
}

inline std::pair<VkAccessFlags, VkAccessFlags> GetAccessFlagsFromLayout(
    VkImageLayout oldLayout, VkImageLayout newLayout) {
    if (oldLayout == newLayout) {
        assert(0);
    }

    VkAccessFlags srcAccessFlags;
    VkAccessFlags dstAccessFlags;

    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            srcAccessFlags = 0;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            srcAccessFlags = VK_ACCESS_MEMORY_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            srcAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            srcAccessFlags = VK_ACCESS_SHADER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            srcAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        default:
            std::cerr << "Not handled layout for access flags translation. (oldLayout)" << std::endl;
            assert(0);
    }

    switch (newLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            dstAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            dstAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            dstAccessFlags = VK_ACCESS_NONE;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            dstAccessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            dstAccessFlags = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            std::cerr << "Not handled layout for access flags translation. (newLayout)" << std::endl;
            assert(0);
    }

    return {srcAccessFlags, dstAccessFlags};
}

inline VkDescriptorType TranslateShaderInputType(ShaderInputType type) {
    switch (type) {
        case ShaderInputType::UNIFORM_BUFFER:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ShaderInputType::TEXTURE:
            return VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default: break;
    }

    std::cerr << "Not handled layout for access flags translation. (newLayout)" << std::endl;
    assert(0);
    return VkDescriptorType::VK_DESCRIPTOR_TYPE_MAX_ENUM;
};
