#pragma once
#include "GPUDefinitions.h"
#include "glm/glm.hpp"

class ITextureInterface;
class GraphicsContext;
class Device;
class RenderTarget;
class Shader;
class RenderPass;

struct GraphicsPipelineParams {
    RasterizationConfiguration _rasterization;
    RenderAttachments _renderAttachments;
    ShaderParams _fsParams;
    ShaderParams _vsParams;
    Device* _device;
    RenderPass* _renderPass;
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

    [[nodiscard]] bool HasDepthAttachments() const { return _params._renderAttachments._depthStencilAttachmentBinding.has_value(); }
    
    GraphicsPipelineParams _params;

protected:
    bool CompileShaders();
    
protected:
    std::unique_ptr<Shader> _fragmentShader;
    std::unique_ptr<Shader> _vertexShader;
};
