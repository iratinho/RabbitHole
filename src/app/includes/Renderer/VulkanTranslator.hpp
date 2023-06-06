#pragma once
#include "vulkan/vulkan.hpp"
#include "GPUDefinitions.h"

namespace {
    VkShaderStageFlagBits TranslateShaderStage(ShaderStage shaderStage) {
        switch (shaderStage) {
            case STAGE_VERTEX:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case STAGE_FRAGMENT:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }

    VkFormat TranslateFormat(Format format) {
        switch (format) {
            case Format::FORMAT_B8G8R8A8_SRGB:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::END_COLOR_FORMATS:
                break;
            case Format::FORMAT_D32_SFLOAT:
                return VK_FORMAT_D32_SFLOAT;
            case Format::FORMAT_R32G32B32_SFLOAT:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case Format::END_DEPTH_FORMATS:
                break;
        }

        return VK_FORMAT_MAX_ENUM;
    }

    VkAttachmentLoadOp TranslateLoadOP(AttachmentLoadOp loadOp) {
        switch (loadOp) {
            case LoadStoreOp::OP_CLEAR:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case LoadStoreOp::OP_LOAD:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case LoadStoreOp::OP_DONT_CARE:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }

        return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
    }

    VkAttachmentStoreOp TranslateStoreOP(AttachmentStoreOp storeOp) {
        switch (storeOp) {
            case StoreOp::OP_STORE:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case StoreOp::OP_DONT_CARE:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        }

        return VK_ATTACHMENT_STORE_OP_MAX_ENUM;
    }

    VkCullModeFlagBits TranslateCullMode(TriangleCullMode cullMode) {
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

    VkFrontFace TranslateWindingOrder(TriangleWindingOrder windingOrder) {
        switch (windingOrder) {
            case WindingOrder::CLOCK_WISE:
                return VkFrontFace ::VK_FRONT_FACE_CLOCKWISE;
            case WindingOrder::COUNTER_CLOCK_WISE:
                return VkFrontFace ::VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }

        return VkFrontFace::VK_FRONT_FACE_MAX_ENUM;
    }

    VkBlendFactor TranslateBlendFactor(BlendFactor blendFactor) {
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

    VkBlendOp TranslateBlendOperation(BlendOperation blendOperation) {
        switch (blendOperation) {
            case BlendOperation::BLEND_OP_ADD:
                return VK_BLEND_OP_ADD;
        }

        return VkBlendOp::VK_BLEND_OP_MAX_ENUM;
    }

    VkImageLayout TranslateImageLayout(ImageLayout layout) {
        switch (layout) {
            case ImageLayout::LAYOUT_UNDEFINED:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::LAYOUT_COLOR_ATTACHMENT:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT:
                return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            case ImageLayout::LAYOUT_PRESENT:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            default:
                return VK_IMAGE_LAYOUT_UNDEFINED;
        }
        
        return VK_IMAGE_LAYOUT_MAX_ENUM;
    }
}
