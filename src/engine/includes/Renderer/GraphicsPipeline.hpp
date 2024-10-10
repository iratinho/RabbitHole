#pragma once
#include "GPUDefinitions.h"
#include "glm/glm.hpp"

class ITextureInterface;
class GraphicsContext;
class Device;
class RenderTarget;
class Shader;


struct GraphicsPipelineParams {
    RasterizationConfiguration _rasterization;
    RenderAttachments _renderAttachments;
    ShaderParams _fsParams;
    ShaderParams _vsParams;
    Device* _device;
    int _id;
};

class GraphicsPipeline {
public:
    GraphicsPipeline(const GraphicsPipelineParams& params);
    
    virtual ~GraphicsPipeline();
    
    /**
     * Creates a new graphics pipeline
     * @param graphicsContext
     */
    static std::shared_ptr<GraphicsPipeline> Create(const GraphicsPipelineParams& params);
    
    /**
     * @brief Compile the pipeline
     *
     */
    virtual void Compile() {};
    
    Shader* GetVertexShader();
    
    Shader* GetFragmentShader();

    [[nodiscard]] bool HasColorAttachments() const { return _params._renderAttachments._colorAttachmentBinding.has_value(); }
    [[nodiscard]] bool HasDepthAttachments() const { return _params._renderAttachments._depthStencilAttachmentBinding.has_value(); }
    
public:
    template <typename T>
    void SetPushConstantValue(std::string name, T value) {
        using Type = PushConstantDataInfo<T>::type;
        constexpr PushConstantDataInfo<T> info;
    
        Type* dataPointer = reinterpret_cast<Type*>((&_pushConstants[name]) + info.offset);
        *dataPointer = value;
    };
    
protected:
    void BuildShaders();
    
protected:
    std::map<std::string, PushConstant> _pushConstants;
    GraphicsPipelineParams _params;
    
    std::unique_ptr<Shader> _fragmentShader;
    std::unique_ptr<Shader> _vertexShader;
};
