#pragma once
#include "CommandEncoders/BlitCommandEncoder.hpp"
#include "Core/Utils.hpp"
#include "entt/entity/entity.hpp"
#include "glm/glm.hpp"

class RenderTarget;
class Texture;
class Texture2D;
class TextureResource;
class Buffer;
class Scene;
class RenderCommandEncoder;
class BlitCommandEncoder;

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

// Should we replace by ImageUsage? It will make it more generic with metal
enum class ImageLayout {
    LAYOUT_UNDEFINED,
    LAYOUT_COLOR_ATTACHMENT,
    LAYOUT_DEPTH_STENCIL_ATTACHMENT,
    LAYOUT_TRANSFER_DST,
    LAYOUT_SHADER_READ,
    LAYOUT_PRESENT
};

enum ShaderStage : unsigned char {
    STAGE_UNDEFINED = 0,
    STAGE_VERTEX = 1,
    STAGE_FRAGMENT = 2
};

enum PushConstantDataType {
    PCDT_Undefined,
    PCDT_Float,
    PCDT_Vec3,
    PCDT_Vec4,
    PCDT_Mat4,
    PCDT_ContiguosMemory
};

template <typename T>
struct PushConstantDataInfo {
    using _isSpecialization = std::false_type;
};

template <>
struct PushConstantDataInfo<float> {
    static const size_t _gpuSize = CalculateGPUDStructSize<float>();
    static const PushConstantDataType _dataType = PCDT_Float;
    using type = float;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::vec3> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::vec3>();
    static const PushConstantDataType _dataType = PCDT_Vec3;
    using type = glm::vec3;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::vec4> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::vec4>();
    static const PushConstantDataType _dataType = PCDT_Vec4;
    using type = glm::vec3;
    using _isSpecialization = std::true_type;
};

template <>
struct PushConstantDataInfo<glm::mat4> {
    static const size_t _gpuSize = CalculateGPUDStructSize<glm::mat4>();
    static const PushConstantDataType _dataType = PCDT_Mat4;
    using type = glm::mat4;
    using _isSpecialization = std::true_type;
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

enum class TextureFilter {
    NEAREST,
    LINEAR
};

struct Sampler {
    TextureFilter _magFilter = TextureFilter::NEAREST;
    TextureFilter _minFilter = TextureFilter::NEAREST;
    TextureFilter _mipMapFilter = TextureFilter::NEAREST;
    TextureWrapMode _wrapU = TextureWrapMode::REPEAT;
    TextureWrapMode _wrapV = TextureWrapMode::REPEAT;
    TextureWrapMode _wrapW = TextureWrapMode::REPEAT;
};

/// Holds information about a texture to be sampled in the shader
struct TextureSampler {
    std::shared_ptr<RenderTarget> _renderTarget;
    Sampler _sampler;
};

enum Format {
    FORMAT_UNDEFINED,
    FORMAT_B8G8R8A8_SRGB,
    FORMAT_B8G8R8A8_UNORM,
    FORMAT_R8G8B8A8_SRGB,
    FORMAT_R8G8B8_SRGB,
    FORMAT_R8G8_SNORM,
    FORMAT_R8G8B8_SNORM,
    FORMAT_R32G32_UNORM,
    END_COLOR_FORMATS, // DO NOT USE, JUST FOR REFERENCE
    FORMAT_D32_SFLOAT,
    END_DEPTH_FORMATS,
    FORMAT_R32G32B32_SFLOAT,
    FORMAT_R32G32_SFLOAT
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


struct ShaderInput {
    Format _format;
    unsigned int _location;
    unsigned int _bufferOffset; // actual data offset in the buffer for this data input
    unsigned int _memberOffset; // offset of the data member. for example a struct member
    bool _bEnabled = false;
};

enum class ShaderInputType {
    UNIFORM_BUFFER,
    TEXTURE
};

struct ShaderBufferResource {
    std::shared_ptr<Buffer> _bufferResource;
    std::size_t _offset = 0; // Offset where the relevant data from the buffer starts
};

struct ShaderTextureResource {
    std::shared_ptr<Texture2D> _texture;
    Sampler _sampler; // TODO separate sampler from texture resource 
};

struct ShaderAttributeBinding {
    unsigned int _binding;
    unsigned int _stride;
    // input rate
    
    bool operator<(const ShaderAttributeBinding& rhs) const {
        return this->_binding < rhs._binding;
    }
    
    bool operator==(const ShaderAttributeBinding& rhs) const {
        return this->_binding == rhs._binding && this->_stride == rhs._stride;
    }
};

using ShadarDataValue = std::variant<glm::mat4, glm::vec4, glm::vec3, float, ShaderTextureResource, ShaderBufferResource>;

enum class ShaderDataBlockUsage {
    NONE, // case for push constants where this is not relevant
    UNIFORM_BUFFER,
    TEXTURE,
    SAMPLER
};

struct ShaderDataBlock {
    PushConstantDataType _type;
    std::size_t _size;
    ShadarDataValue _data; // Bound at runtime
    ShaderDataBlockUsage _usage;
    ShaderStage _stage;
    std::string _identifier;
};

enum class ShaderDataStreamUsage {
    PUSH_CONSTANT,
    DATA
};

struct ShaderDataStream {
    std::vector<ShaderDataBlock> _dataBlocks; // todo make vector of vector to create multiple binding points based on the same data stream layout
    //ShaderStage _stage; // DEPDRECATE
    ShaderDataStreamUsage _usage;
    std::uint8_t _binding;
};

namespace std {
    // Provide the hash operator for ShaderInputBinding, so that we can use with hash containers
    template <>
    struct std::hash<ShaderAttributeBinding> {
        std::size_t operator()(const ShaderAttributeBinding& key) const {
            std::size_t hash1 = std::hash<int>{}(key._binding);
            std::size_t hash2 = std::hash<int>{}(key._stride);
            
            std::size_t seed = 0;
            std::hash<size_t> hasher;
            
            seed ^= hasher(hash1) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            seed ^= hasher(hash2) + 0x9e3779b9 + (seed<<6) + (seed>>2);
            
            return seed;
        };
    };
};


struct ShaderInputLocation {
    Format _format;
    unsigned int _offset;
};

using ShaderInputBindings = std::unordered_map<ShaderAttributeBinding, std::vector<ShaderInputLocation>>;

struct PrimitiveProxy {
    unsigned int _indicesBufferOffset;
    unsigned int _indicesCount;
    std::vector<unsigned int> _vOffset;
    std::shared_ptr<Buffer> _primitiveBuffer;
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
    Tex_PRESENTATION = 1 << 4,
    Tex_DYNAMIC = 1 << 5
};

enum TextureLoadFlags {
    TexLoad_None,
    TexLoad_Path, // Created from external path
    TexLoad_Data, // Created with an array of pixel data
    TexLoad_DynamicData, // Created with no initial data, but pixel data is updated on the fly
    TexLoad_Attachment, // Created with no data, resources will act as attachments for render passes
    TexLoad_ExternalResource // Created with external resource. Ex: swapchain images
};

enum TextureType {
    Texture_2D,
    Texture_3D
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
    ColorAttachmentBlending _blending;
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
    
    ColorAttachmentBinding _colorAttachmentBinding; // We could support multiple color attachments bindings
    std::optional<DepthStencilAttachmentBinding> _depthStencilAttachmentBinding;
};

struct PassResources {
    std::vector<std::shared_ptr<Texture2D>> _textures;
    std::vector<std::shared_ptr<Buffer>> _buffersResources;
};

struct VertexData {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 color;
};

struct RenderPrimitiveInfo {
    entt::entity _entity;
    struct PrimitiveProxyComponent* _proxy;
};

struct ShaderParams {
    ShaderInputBindings _shaderInputBindings;
    std::vector<ShaderDataStream> _shaderDataStreams;
    std::string _shaderPath;
};

struct Encoders {
    RenderCommandEncoder* _renderEncoder;
};

using RasterRenderFunction = std::function<void(Encoders, class GraphicsPipeline* pipeline)>;
using BlitCommandCallback = std::function<void(Encoders, PassResources readResources, PassResources writeResources)>;
