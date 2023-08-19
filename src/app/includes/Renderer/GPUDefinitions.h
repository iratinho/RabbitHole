#pragma once

class RenderTarget;
class Buffer;

typedef enum class CullMode {
    CULL_MODE_NONE,
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_ALL
} TriangleCullMode;

typedef enum class WindingOrder {
    CLOCK_WISE,
    COUNTER_CLOCK_WISE
} TriangleWindingOrder;

typedef enum class BlendFactor {
    BLEND_FACTOR_ZERO,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_COLOR,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
} BlendFactor;

typedef enum class BlendOperation {
    BLEND_OP_ADD
} BlendOperation;

typedef enum class CompareOperation {
    NEVER = 0,
    LESS = 1,
    EQUAL = 2,
    LESS_OR_EQUAL = 3,
    GREATER = 4,
    NOT_EQUAL = 5,
    GREATER_OR_EQUAL = 6,
    ALWAYS = 7
} CompareOperation;

typedef struct RasterizationConfiguration {
    float _depthBias = 0.0f;
    float _depthBiasSlope = 0.0f;
    float _depthBiasClamp = 0.0f;
    CompareOperation _depthCompareOP = CompareOperation::NEVER;
    TriangleCullMode  _triangleCullMode = TriangleCullMode::CULL_MODE_ALL;
    TriangleWindingOrder _triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
} PipelineRasterizationInfo;

typedef struct BlendState {

} ColorAttachmentState;

typedef struct DepthState {

} PipelineDepthState;

typedef enum ShaderStage {
    STAGE_VERTEX,
    STAGE_FRAGMENT
} ShaderStage;

typedef struct PushConstant {
    unsigned int _size;
    ShaderStage _shaderStage;
} PushConstant;

struct PushConstantConfiguration {
    PushConstant _pushConstant;
    std::vector<std::vector<char>> _data;
    size_t size;
    
    // For debug
    std::string _debugType;
    
private:
    int _id = 0;
    friend class RenderPassGenerator;
};

typedef enum class WrapMode {
    REPEAT,
    MIRRORED_REPEAT,
    CLAMP_TO_EDGE,
    CLAMP_TO_BORDER,
    MIRROR_CLAMp_TO_EDGE
} TextureWrapMode;

typedef enum class Filter {
    NEAREST,
    LINEAR,
} MipMapFilter;

typedef enum class Filter2 {
    NEAREST,
    LINEAR,
    CUBIC
} MinificationFilter, MagnificationFilter;

typedef struct Sampler {
    MagnificationFilter _magFilter;
    MinificationFilter _minFilter;
    MipMapFilter _mipMapFilter;
    TextureWrapMode _wrapU;
    TextureWrapMode _wrapV;
    TextureWrapMode _wrapW;
} Sampler;

typedef enum class Format {
    FORMAT_UNDEFINED,
    FORMAT_B8G8R8A8_SRGB,
    END_COLOR_FORMATS, // DO NOT USE, JUST FOR REFERENCE
    FORMAT_D32_SFLOAT,
    END_DEPTH_FORMATS,
    FORMAT_R32G32B32_SFLOAT
} AttachmentFormat, ImageFormat;

typedef enum class LoadStoreOp {
    OP_CLEAR,
    OP_LOAD,
    OP_DONT_CARE
} AttachmentLoadOp, AttachmentStencilLoadOp;

typedef enum class StoreOp {
    OP_STORE,
    OP_DONT_CARE
} AttachmentStoreOp, AttachmentStencilStoreOp;

typedef struct Attachment {
    unsigned int _sampleCount = 1;
    AttachmentFormat _format;
    AttachmentLoadOp _loadOp;
    AttachmentStoreOp _storeOp;
    AttachmentStencilLoadOp _stencilLoadOp;
    AttachmentStencilStoreOp _stencilStoreOp;

    bool bBlendEnabled;
    BlendFactor _srcColorBlendFactor;
    BlendFactor _dstColorBlendFactor;
    BlendFactor _srcAlphaBlendFactor;
    BlendFactor _dstAlphaBlendFactor;
    BlendOperation _colorBlendOp;
    BlendOperation _alphaBlendOp;
} ColorAttachment, DepthAttachment;

typedef enum class ImageLayout {
    LAYOUT_UNDEFINED,
    LAYOUT_COLOR_ATTACHMENT,
    LAYOUT_DEPTH_STENCIL_ATTACHMENT,
    LAYOUT_PRESENT
} ImageLayout;

struct AttachmentConfiguration {
    std::shared_ptr<RenderTarget> _renderTarget;
    Attachment _attachment;
    
    // Only have meaning for vulkan
    ImageLayout _initialLayout = ImageLayout::LAYOUT_UNDEFINED;
    ImageLayout _finalLayout = ImageLayout::LAYOUT_UNDEFINED;
};

struct ShaderInput {
    Format _format;
    unsigned int _location;
    unsigned int _bufferOffset; // actual data offset in the buffer for this data input
    unsigned int _memberOffset; // offset of the data member. for example a struct member
    bool _bEnabled = false;
};

// Relates to a bind (data stream in a buffer) where it contains multiple InputDescriptors
// that specifies a region inside a larger group
// In vulkan this is the same as VkVertexInputBindingDescription
// refer to this for better understanding
// https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/vertex_input_data_processing.adoc
struct ShaderInputGroup {
    unsigned int _binding;
    unsigned int _stride;
    unsigned int _offset;
    std::vector<ShaderInput> _inputDescriptors;
};

struct ShaderConfiguration {
    const char* _shaderPath;
//    std::unordered_map<std::string, PushConstantConfiguration> _pushConstants;
};

struct PrimitiveProxy {
    unsigned int _indicesBufferOffset;
    unsigned int _indicesCount;
    std::vector<unsigned int> _vOffset;
    std::shared_ptr<Buffer> _primitiveBuffer;
    std::vector<PushConstantConfiguration> _pushConstants;
};
