#pragma once
#include "Renderer/GPUDefinitions.h"
#include "webgpu/webgpu.hpp"

// TODO Translate pixel format and vertex format instead of this general format, both vulkan and webgpu

inline WGPUTextureFormat TranslateFormat(Format format) {
    switch (format) {
        case Format::FORMAT_R8G8_SNORM:
            return WGPUTextureFormat::WGPUTextureFormat_RG8Snorm;
        case Format::FORMAT_B8G8R8A8_SRGB:
            return WGPUTextureFormat::WGPUTextureFormat_BGRA8UnormSrgb;
        case Format::FORMAT_R8G8B8A8_SRGB:
            return WGPUTextureFormat::WGPUTextureFormat_RGBA8UnormSrgb;
        case Format::FORMAT_B8G8R8A8_UNORM:
            return WGPUTextureFormat::WGPUTextureFormat_BGRA8Unorm;
        case Format::END_COLOR_FORMATS:
            break;
        case Format::FORMAT_D32_SFLOAT:
            return WGPUTextureFormat::WGPUTextureFormat_Depth32Float;
        case Format::FORMAT_R32G32B32_SFLOAT:
            return WGPUTextureFormat::WGPUTextureFormat_RGBA32Float;
        case Format::FORMAT_R32G32_SFLOAT:
            return WGPUTextureFormat::WGPUTextureFormat_RG32Float;
        case Format::END_DEPTH_FORMATS:
            break;
        case Format::FORMAT_UNDEFINED:
            return WGPUTextureFormat::WGPUTextureFormat_Undefined;
        default: break;
    }

    assert(0 && "Invalid WebGPU format translation");
    return WGPUTextureFormat_Undefined;
}

inline WGPUVertexFormat TranslateVertexFormat(Format format) {
    switch (format) {
        case FORMAT_R32G32_SFLOAT:
            return WGPUVertexFormat::WGPUVertexFormat_Float32x2;
        case FORMAT_R32G32B32_SFLOAT:
            return WGPUVertexFormat::WGPUVertexFormat_Float32x3;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU vertex format translation");
    return WGPUVertexFormat::WGPUVertexFormat_Undefined;
}

inline WGPULoadOp TranslateLoadOP(LoadOp loadOp) {
    switch (loadOp) {
        case LoadOp::OP_CLEAR:
            return WGPULoadOp::WGPULoadOp_Clear;
        case LoadOp::OP_LOAD:
            return WGPULoadOp::WGPULoadOp_Load;
    }

    assert(0 && "Invalid WebGPU LoadOp translation");
    return WGPULoadOp::WGPULoadOp_Undefined;
}

inline WGPUBlendFactor TranslateBlendFactor(BlendFactor blendFactor) {
    switch (blendFactor) {
        case BlendFactor::BLEND_FACTOR_ONE:
            return WGPUBlendFactor::WGPUBlendFactor_One;
        case BlendFactor::BLEND_FACTOR_ZERO:
            return WGPUBlendFactor::WGPUBlendFactor_Zero;
        case BlendFactor::BLEND_FACTOR_SRC_ALPHA:
            return WGPUBlendFactor::WGPUBlendFactor_SrcAlpha;
        case BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return WGPUBlendFactor::WGPUBlendFactor_OneMinusSrcAlpha;
        case BlendFactor::BLEND_FACTOR_SRC_COLOR:
            return WGPUBlendFactor::WGPUBlendFactor_Src;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU BlendFactor translation");
    return WGPUBlendFactor::WGPUBlendFactor_Zero;
}

inline WGPUBlendOperation TranslateBlendOperation(BlendOperation blendOperation) {
    switch (blendOperation) {
        case BlendOperation::BLEND_OP_ADD:
            return WGPUBlendOperation::WGPUBlendOperation_Add;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU BlendOperation translation");
    return WGPUBlendOperation::WGPUBlendOperation_Max;
}

inline WGPUCompareFunction TranslateCompareOP(CompareOperation compareOperation) {
    switch (compareOperation) {
        case CompareOperation::NEVER:
            return WGPUCompareFunction::WGPUCompareFunction_Never;
        case CompareOperation::LESS:
            return WGPUCompareFunction::WGPUCompareFunction_Less;
        case CompareOperation::EQUAL:
            return WGPUCompareFunction::WGPUCompareFunction_Equal;
        case CompareOperation::LESS_OR_EQUAL:
            return WGPUCompareFunction::WGPUCompareFunction_LessEqual;
        case CompareOperation::GREATER:
            return WGPUCompareFunction::WGPUCompareFunction_Greater;
        case CompareOperation::NOT_EQUAL:
            return WGPUCompareFunction::WGPUCompareFunction_NotEqual;
        case CompareOperation::GREATER_OR_EQUAL:
            return WGPUCompareFunction::WGPUCompareFunction_GreaterEqual;
        case CompareOperation::ALWAYS:
            return WGPUCompareFunction::WGPUCompareFunction_Always;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU CompareOperation translation");
    return WGPUCompareFunction::WGPUCompareFunction_Never;
}

inline WGPUFrontFace TranslateWindingOrder(WindingOrder windingOrder) {
    switch (windingOrder) {
        case WindingOrder::CLOCK_WISE:
            return WGPUFrontFace::WGPUFrontFace_CW;
        case WindingOrder::COUNTER_CLOCK_WISE:
            return WGPUFrontFace::WGPUFrontFace_CCW;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU WindingOrder translation");
    return WGPUFrontFace::WGPUFrontFace_CW;
}

inline WGPUCullMode TranslateCullMode(CullMode cullMode) {
    switch (cullMode) {
        case CullMode::CULL_MODE_ALL:
            return WGPUCullMode::WGPUCullMode_Force32; //??
        case CullMode::CULL_MODE_FRONT:
            return WGPUCullMode::WGPUCullMode_Back;
        case CullMode::CULL_MODE_BACK:
            return WGPUCullMode::WGPUCullMode_Front;
        case CullMode::CULL_MODE_NONE:
            return WGPUCullMode::WGPUCullMode_None;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU CullMode translation");
    return WGPUCullMode::WGPUCullMode_None;
}

inline WGPUShaderStage TranslateShaderStage(ShaderStage stage) {
    if(stage & ShaderStage::STAGE_VERTEX && stage & ShaderStage::STAGE_FRAGMENT) {
        return (WGPUShaderStage)(WGPUShaderStage::WGPUShaderStage_Vertex | WGPUShaderStage::WGPUShaderStage_Fragment);
    }
    
    switch (stage) {
        case ShaderStage::STAGE_VERTEX:
            return WGPUShaderStage::WGPUShaderStage_Vertex;
        case ShaderStage::STAGE_FRAGMENT:
            return WGPUShaderStage::WGPUShaderStage_Fragment;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU CullMode translation");
    return WGPUShaderStage::WGPUShaderStage_None;
}

inline WGPUTextureUsageFlags TranslateTextureUsageFlags(TextureFlags flags) {
    if(flags & TextureFlags::Tex_SAMPLED_OP && flags & TextureFlags::Tex_TRANSFER_DEST_OP) {
        return WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    }
    
    if(flags & TextureFlags::Tex_COLOR_ATTACHMENT && flags & TextureFlags::Tex_TRANSFER_SRC_OP) {
        return WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    }
    
    switch (flags) {
        case TextureFlags::Tex_SAMPLED_OP:
            return WGPUTextureUsage_TextureBinding;
        case TextureFlags::Tex_DEPTH_ATTACHMENT:
        case TextureFlags::Tex_COLOR_ATTACHMENT:
            return WGPUTextureUsage_RenderAttachment;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU TextureFlags translation");
    return WGPUTextureUsage_None;
}

inline WGPULoadOp TranslateLoadOp(LoadOp loadOp) {
    switch (loadOp) {
        case LoadOp::OP_LOAD:
            return WGPULoadOp::WGPULoadOp_Load;
        case LoadOp::OP_CLEAR:
            return WGPULoadOp::WGPULoadOp_Clear;
        case LoadOp::OP_DONT_CARE:
            return WGPULoadOp::WGPULoadOp_Undefined; // ??
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU LoadOp translation");
    return WGPULoadOp::WGPULoadOp_Undefined;
}

inline WGPUAddressMode TranslateWrapMode(WrapMode wrapMode) {
    switch (wrapMode) {
        case WrapMode::CLAMP_TO_EDGE:
            return WGPUAddressMode::WGPUAddressMode_ClampToEdge;
        case WrapMode::REPEAT:
            return WGPUAddressMode::WGPUAddressMode_Repeat;
        case WrapMode::MIRRORED_REPEAT:
            return WGPUAddressMode::WGPUAddressMode_MirrorRepeat;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU WrapMode translation");
    return {};
}

inline WGPUFilterMode TranslateTextureFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::LINEAR:
            return WGPUFilterMode::WGPUFilterMode_Linear;
        case TextureFilter::NEAREST:
            return WGPUFilterMode::WGPUFilterMode_Nearest;
        default:
            break;
    }
    
    assert(0 && "Invalid WebGPU TextureFilter translation");
    return {};
}
