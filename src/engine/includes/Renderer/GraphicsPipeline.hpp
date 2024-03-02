#pragma once
#include "Renderer/Shader.hpp"
#include "GPUDefinitions.h"
#include "glm/glm.hpp"
#include <map>

class ITextureInterface;
class GraphicsContext;
class RenderContext;
class RenderTarget;

struct GraphicsPipelineParams {
    std::shared_ptr<Shader> _fragmentShader;
    std::shared_ptr<Shader> _vertexShader;
    RasterizationConfiguration _rasterization;
    Attachment _colorAttachment;
    Attachment _depthAttachment;
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
     * @brief Set the Shader object
     *
     * @param shaderStage
     * @param shader
     * @return GraphicsPipeline&
     */
//    virtual void SetShader(ShaderStage shaderStage, std::unique_ptr<Shader>&& shader) = 0;
    
    /**
     * @brief 
     * 
     * @return GraphicsPipeline& 
     */
    virtual void BeginVertexInput() = 0;

    /**
     * @brief Set the Vertex Inputs object
     *
     * @param inputGroup
     */
    virtual void SetVertexInputs(const std::vector<ShaderInputGroup> &inputGroup) = 0;
    
    /**
     * @brief Set a shader vertex input
     *
     * @param binding
     * @param location
     */
    virtual void SetVertexInput(const ShaderInputBinding& binding, const ShaderInputLocation& location) = 0;

    /**
     * @brief Set the Rasterization Params object
     *
     * @param rasterizationConfiguration
     * @return GraphicsPipeline&
     */
    virtual void SetRasterizationParams(const RasterizationConfiguration &rasterizationConfiguration) = 0;

    /**
     * @brief Declares a new texture that is going to be sampled in the shaders
     *
     * @param textureSampler
     * @return GraphicsPipeline&
     */
    virtual void DeclareSampler(std::shared_ptr<Texture2D> textureSampler) = 0;

    /**
     * @brief Compile the pipeline
     *
     */
    virtual void Compile() = 0;
    
    /**
     * @brief Draws a 3d primitive
     *
     * @param proxy that contains rendering information about a mesh primitive
     */
    virtual void Draw(const PrimitiveProxy& proxy) = 0;
    
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
