#pragma once
#include "GPUDefinitions.h"

class GraphicsContext;
class GraphicsPipeline;
class RenderContext;

class Shader {
private:
    using PushConstants = std::vector<PushConstant>;
    using InputAttributes = std::unordered_map<ShaderAttributeBinding, std::vector<ShaderInputLocation>>;
    using ShaderInputs = std::vector<ShaderResourceBinding>;
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
    
    Shader(RenderContext* device, GraphicsPipeline* pipeline, ShaderStage stage,ShaderParams params)
    : _stage(stage)
    , _params(params)
    , _device(device)
    , _pipeline(pipeline)
    {
        _constants = params._shaderInputConstants;
        _shaderInputs = params._shaderResourceBindings;
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
        _constants = rhs._constants;
        _inputAttr = rhs._inputAttr;
        _device = rhs._device;
        _pipeline = rhs._pipeline;
        
        return *this;
    }
    
    Shader& operator=(Shader&& rhs) noexcept {
        _graphicsContext = rhs._graphicsContext;
        _path = std::move(rhs._path);
        _stage = rhs._stage;
        _constants = std::move(rhs._constants);
        _inputAttr = std::move(rhs._inputAttr);
        _pipeline = rhs._pipeline;
        
        rhs._graphicsContext = nullptr;
        rhs._pipeline = nullptr;
        rhs._stage = ShaderStage::STAGE_UNDEFINED;
                
        return *this;
    }
    
    static std::shared_ptr<Shader> MakeShader(GraphicsContext* _graphicsContext, const std::string& path, ShaderStage stage);
    
    static std::unique_ptr<Shader> MakeShader(RenderContext* device, GraphicsPipeline* pipeline, ShaderStage stage, const ShaderParams& params);
    
    static std::shared_ptr<Shader> GetShader(GraphicsContext* _graphicsContext, const std::string& path, ShaderStage stage);
        
    ShaderResourceBinding GetShaderResourceBinding(const std::string& identifier) {
        auto shaderInput = std::find_if(_shaderInputs.begin(), _shaderInputs.end(), [&](const ShaderResourceBinding& v) {
            return v._identifier == identifier;
        });

        if(shaderInput == _shaderInputs.end()) {
            assert(0);
            return {};
        }
        
        return *shaderInput;
    }
    
    /**
     * @brief - Compiles the current shader
     * @returns - returns if the shader was compiled
     */
    virtual bool Compile() = 0;

    std::size_t GetHash() {
        return hash_value(_path);
    };
    

    void DeclareShaderInput(ShaderResourceBinding param) {
        _shaderInputs.push_back(param);
    }
        
    [[nodiscard]] inline ShaderInputs& GetShaderInputs() {
        return _shaderInputs;
    }

    /**
     * @brief Specifies a shader binding layout (only makes sense when using vertex shader)
     * @param binding - The binding point description
     * @param layouts - A list of shader layouts for the current binding
     */
    void DeclareShaderBindingLayout(ShaderAttributeBinding binding, const std::vector<ShaderInputLocation>& layouts) {
        _inputAttr[binding] = layouts;
    };
    
    template <typename Value, typename = std::enable_if_t<PushConstantDataInfo<Value>::_isSpecialization::value>>
    void DeclarePushConstant(const std::string& name) {
        constexpr PushConstantDataInfo<Value> info;
        
        // TODO make constanss as a map
        for(auto constant : _constants) {
            if(constant.name == name)
                return;
        }
        
        PushConstant pushConstant;
        pushConstant.name = name;
        pushConstant._dataType = info._dataType;
        pushConstant._size = info._gpuSize;
        pushConstant._shaderStage = _stage;
        
        _constants.push_back(pushConstant);
    };
        
    [[nodiscard]] inline const PushConstants& GetConstants() const {
        return _constants;
    };
    
    [[nodiscard]] inline const InputAttributes& GetInputAttributes() const {
        return _inputAttr;
    };
    
    [[nodiscard]] inline const ShaderStage GetShaderStage() const {
        return _stage;
    };
    
    [[nodiscard]] inline const GraphicsPipeline* GetPipeline() const {
        return _pipeline;
    }
        
    GraphicsContext* _graphicsContext       = nullptr;
    
protected:
    ShaderStage _stage                  = ShaderStage::STAGE_UNDEFINED;
    std::string _path;
    PushConstants _constants;
    InputAttributes _inputAttr;
    ShaderInputs _shaderInputs;
    ShaderParams _params;
    RenderContext* _device;
    GraphicsPipeline* _pipeline;
};
