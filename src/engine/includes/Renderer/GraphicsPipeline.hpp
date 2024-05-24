#pragma once
#include "Renderer/Shader.hpp"
#include "GPUDefinitions.h"
#include "glm/glm.hpp"

class ITextureInterface;
class GraphicsContext;
class RenderContext;
class RenderTarget;

struct GraphicsPipelineParams {
    std::shared_ptr<Shader> _fragmentShader;
    std::shared_ptr<Shader> _vertexShader;
    RasterizationConfiguration _rasterization;
    RenderAttachments _renderAttachments;
    GraphicsContext* _graphicsContext;
    int _id;
};

class GraphicsPipeline {
public:
    GraphicsPipeline(const GraphicsPipelineParams& params)
        : _params(params) {
    };
    
    virtual ~GraphicsPipeline() {};
    
    /**
     * Creates a new graphics pipeline
     * @param graphicsContext
     */
    static std::shared_ptr<GraphicsPipeline> Create(const GraphicsPipelineParams& params);
    
    /**
     * @brief Compile the pipeline
     *
     */
    virtual void Compile() = 0;
    
    
    /**
     * @brief Returns the associated graphics context with this pipeline
     */
    GraphicsContext* GetGraphicsContext() {
        return _params._graphicsContext;
    };
    
public:
    template <typename T>
    void SetPushConstantValue(std::string name, T value) {
        using Type = PushConstantDataInfo<T>::type;
        constexpr PushConstantDataInfo<T> info;
    
        Type* dataPointer = reinterpret_cast<Type*>((&_pushConstants[name]) + info.offset);
        *dataPointer = value;
    };
    
protected:
    std::map<std::string, PushConstant> _pushConstants;
    GraphicsPipelineParams _params;
};
