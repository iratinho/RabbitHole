#pragma once
#include "Core/Utils.hpp"
#include "glm/glm.hpp"

class RenderTarget;
class Texture;
class Texture2D;
class TextureResource;
class Buffer;
class Scene;

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

// Should we replace by ImageUsage? It will make it more generic with metal
enum class ImageLayout {
    LAYOUT_UNDEFINED,
    LAYOUT_COLOR_ATTACHMENT,
    LAYOUT_DEPTH_STENCIL_ATTACHMENT,
    LAYOUT_PRESENT
};

enum ShaderStage : unsigned char {
    STAGE_UNDEFINED,
    STAGE_VERTEX,
    STAGE_FRAGMENT
};

enum PushConstantDataType {
    PCDT_Undefined,
    PCDT_Float,
    PCDT_Vec3,
    PCDT_Vec4,
    PCDT_Mat4,
};

struct PushConstant
{
    std::string name;
    ShaderStage _shaderStage;
    PushConstantDataType _dataType;
    size_t _size = 0;
    
    union {
        float _1;
        float* _2;
        glm::vec3 _vec3;
        glm::vec4 _vec4;
    };
};

template <typename T>
struct PushConstantDataInfo {
    using _isSpecialization = std::false_type;
};

template <>
struct PushConstantDataInfo<float> {
    static const size_t _gpuSize = CalculateGPUDStructSize<float>();
    static const size_t _offset = offsetof(PushConstant, _1);
    static const PushConstantDataType _dataType = PCDT_Float;
    using type = float;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::vec3> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::vec3>();
    static const size_t _offset = offsetof(PushConstant, _vec3);
    static const PushConstantDataType _dataType = PCDT_Vec3;
    using type = glm::vec3;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::vec4> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::vec4>();
    static const size_t _offset = offsetof(PushConstant, _vec4);
    static const PushConstantDataType _dataType = PCDT_Vec4;
    using type = glm::vec3;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::mat4> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::mat4>();
    static const size_t _offset = offsetof(PushConstant, _2);
    static const PushConstantDataType _dataType = PCDT_Mat4;
    using type = glm::mat4;
    using _isSpecialization = std::true_type;
};

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
    MIRROR_CLAMP_TO_EDGE
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

struct Sampler {
    MagnificationFilter _magFilter;
    MinificationFilter _minFilter;
    MipMapFilter _mipMapFilter;
    TextureWrapMode _wrapU;
    TextureWrapMode _wrapV;
    TextureWrapMode _wrapW;
};

/// Holds information about a texture to be sampled in the shader
struct TextureSampler {
    std::shared_ptr<RenderTarget> _renderTarget;
    Sampler _sampler;
};

enum Format {
    FORMAT_UNDEFINED,
    FORMAT_B8G8R8A8_SRGB,
    FORMAT_R8G8B8A8_SRGB,
    END_COLOR_FORMATS, // DO NOT USE, JUST FOR REFERENCE
    FORMAT_D32_SFLOAT,
    END_DEPTH_FORMATS,
    FORMAT_R32G32B32_SFLOAT
};

enum class LoadOp {
    OP_CLEAR,
    OP_LOAD,
    OP_DONT_CARE
};

enum class StoreOp {
    OP_STORE,
    OP_DONT_CARE
};

struct LoadStoreOps {
    LoadOp load;
    StoreOp store;
    LoadOp stencilLoad;
    StoreOp stencilStore;
};

struct BlendFactorOps {
    BlendFactor srcFactor;
    BlendFactor destFactor;
    BlendFactor alphaSrcFactor;
    BlendFactor alphaDestFactor;
};

struct BlendOps {
    BlendOperation colorOp;
    BlendOperation alphaOp;
};

struct LayoutOp {
    ImageLayout _oldLayout;
    ImageLayout _newLayout;
};

struct AccessOp {
};

struct Ops {
public:
    enum Types {
        LOAD_STORE,
        BLEND_FACTOR,
        BLEND,
        LAYOUT
    };
    
    union {
        LoadStoreOps _loadStoreOp;
        BlendFactorOps _blendFactorOp;
        BlendOps _blendOp;
        LayoutOp _layoutOp;
        AccessOp _accessOp;
    };
    
private:
    template <Types op>
    struct TypeData {};
    
    template <>
    struct TypeData<Types::LOAD_STORE> {
        using RetType = LoadStoreOps;
        static LoadStoreOps Ops::* GetMember() { return &Ops::_loadStoreOp; }
    };
    
    template <>
    struct TypeData<Types::BLEND_FACTOR> {
        using RetType = BlendFactorOps;
        static BlendFactorOps Ops::* GetMember() { return &Ops::_blendFactorOp; }
    };
    
    template <>
    struct TypeData<Types::BLEND> {
        using RetType = BlendOps;
        static BlendOps Ops::* GetMember() { return &Ops::_blendOp; }
    };
    
    template <>
    struct TypeData<Types::LAYOUT> {
        using RetType = LayoutOp;
        static LayoutOp Ops::* GetMember() { return &Ops::_layoutOp; }
    };

public:
    template <LoadOp loadOp, StoreOp storeOp, LoadOp stencilLoadOp, StoreOp stencilStoreOp>
    struct StaticOp {
        static Ops Get() {
            Ops ops;
            ops._loadStoreOp.load = loadOp;
            ops._loadStoreOp.store = storeOp;
            ops._loadStoreOp.stencilLoad = stencilLoadOp;
            ops._loadStoreOp.stencilStore = stencilStoreOp;
            return ops;
        }
    };
    
    template <BlendFactor src, BlendFactor dest, BlendFactor alphaSrc, BlendFactor alphaDest>
    struct StaticBlendFactorOp {
        static Ops Get() {
            Ops ops;
            ops._blendFactorOp.srcFactor = src;
            ops._blendFactorOp.destFactor = dest;
            ops._blendFactorOp.alphaSrcFactor = alphaSrc;
            ops._blendFactorOp.alphaDestFactor = alphaDest;
            return ops;
        }
    };

    template <BlendOperation colorOp, BlendOperation alphaOp>
    struct StaticBlendOp {
        static Ops Get() {
            Ops ops;
            ops._blendOp.colorOp = colorOp;
            ops._blendOp.alphaOp = alphaOp;
            return ops;
        }
    };
    
    template <ImageLayout oldLayout, ImageLayout newLayout>
    struct StaticLayoutOp {
        [[nodiscard]] static Ops Get() {
            Ops ops;
            ops._layoutOp._oldLayout = oldLayout;
            ops._layoutOp._newLayout = newLayout;
            return ops;
        }
    };
    
    template <Types type>
    TypeData<type>::RetType GetValue() { return this->*TypeData<type>::GetMember(); }
};



typedef struct Attachment {
    unsigned int _sampleCount = 1;
    Format _format;

    bool bBlendEnabled;

    Ops _loadStoreOp;
    Ops _blendFactorOp;
    Ops _blendOp;
    Ops _layoutOp;
} ColorAttachment, DepthAttachment;

struct RenderPassTexture {
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

enum class ShaderInputType {
    UNIFORM_BUFFER,
    TEXTURE
};

struct ShaderInputParam {
    unsigned int _id;
    ShaderInputType _type;
    ShaderStage _shaderStage;
    std::string _identifier;
};

struct ShaderInputBinding {
    unsigned int _binding;
    unsigned int _stride;
    // input rate
    
    bool operator<(const ShaderInputBinding& rhs) const {
        return this->_binding < rhs._binding;
    }
    
    bool operator==(const ShaderInputBinding& rhs) const {
        return this->_binding == rhs._binding && this->_stride == rhs._stride;
    }
};

struct ShaderInputLocation {
    Format _format;
    unsigned int _offset;
};

struct PrimitiveProxy {
    unsigned int _indicesBufferOffset;
    unsigned int _indicesCount;
    std::vector<unsigned int> _vOffset;
    std::shared_ptr<Buffer> _primitiveBuffer;
    std::vector<PushConstantConfiguration> _pushConstants;
    glm::mat4 _transformMatrix;
    Buffer* _gpuBuffer;
    
    // Abstract this into viewport class
    Scene* _primitiveScene = nullptr;
    unsigned int _viewportX;
    unsigned int _viewportY;
};

enum TextureFlags {
    Tex_None = 0,
    Tex_COLOR_ATTACHMENT = 1 << 0,
    Tex_DEPTH_ATTACHMENT = 1 << 1,
    Tex_TRANSFER_DEST_OP = 1 << 2,
    Tex_SAMPLED_OP = 1 << 3,
    Tex_PRESENTATION = 1 << 4
};

enum TextureType {
    Texture_2D,
    Texture_3D
};

enum class TextureFilter {
    NEAREST,
    LINEAR
};

struct RasterPassTarget {
    std::string _identifier;
    std::shared_ptr<RenderTarget> _renderTarget;
    Attachment _attachmentInfo;
    ImageLayout _targetLayout;
//        Ops _layoutOp; // This Ops is generic code, we dont need to provide oldLayout, we track it with all images
    glm::vec3 _clearColor;
};


struct AttachmentBlendFactor {
    BlendFactor _srcBlendFactor;
    BlendFactor _dstBlendFactor;
};

struct ColorAttachmentBlending {
    BlendOperation _colorBlending;
    BlendOperation _alphaBlending;
    
    AttachmentBlendFactor _colorBlendingFactor;
    AttachmentBlendFactor _alphaBlendingFactor;
};

struct ColorAttachmentBinding {
    std::shared_ptr<Texture2D> _texture;
    std::optional<ColorAttachmentBlending> _blending;
    LoadOp _loadAction;
};

struct DepthStencilAttachmentBinding {
    std::shared_ptr<Texture2D> _texture;
    LoadOp _depthLoadAction;
    LoadOp _stencilLoadAction;
    StoreOp _stencilStoreAction = StoreOp::OP_DONT_CARE;
};

struct RenderAttachments {
    std::string _identifier;
    
    std::optional<ColorAttachmentBinding> _colorAttachmentBinding; // We could support multiple color attachments bindings
    std::optional<DepthStencilAttachmentBinding> _depthStencilAttachmentBinding;
};

struct PassResources {
    std::vector<std::shared_ptr<TextureResource>> _textureResources;
    std::vector<std::shared_ptr<Buffer>> _buffersResources;
};

using CommandCallback = std::function<void(class RenderCommandEncoder*, class GraphicsPipeline* pipeline)>;
using BlitCommandCallback = std::function<void(class RenderCommandEncoder*, PassResources readResources, PassResources writeResources)>;

struct VertexData {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};
