#pragma once
#include "GPUDefinitions.h"

class GraphicsContext;
class GraphicsPipeline;
class Device;
class ShaderSet;

class Shader {
private:
    using InputAttributes = std::unordered_map<ShaderAttributeBinding, std::vector<ShaderInputLocation>>;
    friend class ShaderFactory;
    
public:
    using Type = Shader;
    
    virtual ~Shader() {}
    
    Shader()
    : _graphicsContext(nullptr)
    , _stage(ShaderStage::STAGE_UNDEFINED)
    {}
    
    Shader(GraphicsContext* graphicsContext, const std::string& path, ShaderStage stage)
    : _graphicsContext(graphicsContext)
    , _path(path)
    , _stage(stage)
    {};
    
    Shader(Device* device, GraphicsPipeline* pipeline, ShaderStage stage,ShaderParams params)
    : _stage(stage)
    , _params(params)
    , _device(device)
    , _pipeline(pipeline)
    {
        _inputAttr = params._shaderInputBindings;
    }
    
    Shader(const Shader& shader) {
        *this = shader;
    }
    
    Shader(Shader&& shader) noexcept {
        *this = std::move(shader);
    }
    
    Shader& operator=(const Shader& rhs) {
        _graphicsContext = rhs._graphicsContext;
        _path = rhs._path;
        _stage = rhs._stage;
        _inputAttr = rhs._inputAttr;
        _device = rhs._device;
        _pipeline = rhs._pipeline;
        
        return *this;
    }
    
    Shader& operator=(Shader&& rhs) noexcept {
        _graphicsContext = rhs._graphicsContext;
        _path = std::move(rhs._path);
        _stage = rhs._stage;
        _inputAttr = std::move(rhs._inputAttr);
        _pipeline = rhs._pipeline;
        
        rhs._graphicsContext = nullptr;
        rhs._pipeline = nullptr;
        rhs._stage = ShaderStage::STAGE_UNDEFINED;
                
        return *this;
    }
    
    static std::shared_ptr<Shader> MakeShader(GraphicsContext* _graphicsContext, const std::string& path, ShaderStage stage);
    
    static std::unique_ptr<Shader> MakeShader(Device* device, GraphicsPipeline* pipeline, ShaderStage stage, const ShaderParams& params);
    
    static std::unique_ptr<Shader> MakeShader(Device* device, GraphicsPipeline* pipeline, ShaderStage stage, ShaderSet* shaderSet);
    
    static std::shared_ptr<Shader> GetShader(GraphicsContext* _graphicsContext, const std::string& path, ShaderStage stage);
            
    /**
     * @brief - Compiles the current shader
     * @returns - returns if the shader was compiled
     */
    virtual bool Compile() = 0;

    std::size_t GetHash() {
        return hash_value(_path);
    };
                                
    [[nodiscard]] inline const ShaderStage GetShaderStage() const {
        return _stage;
    };
    
    [[nodiscard]] inline const GraphicsPipeline* GetPipeline() const {
        return _pipeline;
    }
        
    GraphicsContext* _graphicsContext       = nullptr;
    
protected:
    std::string _path;
    InputAttributes _inputAttr;
    ShaderParams _params;
    Device* _device;
    GraphicsPipeline* _pipeline;
    ShaderStage _stage = ShaderStage::STAGE_UNDEFINED;
};
