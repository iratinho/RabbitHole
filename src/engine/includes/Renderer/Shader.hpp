#pragma once
#include "GPUDefinitions.h"

class GraphicsContext;

namespace std {
    // Provide the hash operator for ShaderInputBinding, so that we can use with hash containers
    template <>
    struct std::hash<ShaderInputBinding> {
        std::size_t operator()(const ShaderInputBinding& key) const {
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

class Shader {
private:
    using PushConstants = std::vector<PushConstant>;
    using InputAttributes = std::unordered_map<ShaderInputBinding, std::vector<ShaderInputLocation>>;
    using ShaderInputs = std::vector<ShaderInputParam>;
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
        
        return *this;
    }

    Shader& operator=(Shader&& rhs) noexcept {
        _graphicsContext = rhs._graphicsContext;
        _path = std::move(rhs._path);
        _stage = rhs._stage;
        _constants = std::move(rhs._constants);
        _inputAttr = std::move(rhs._inputAttr);
        
        rhs._graphicsContext = nullptr;
        rhs._stage = ShaderStage::STAGE_UNDEFINED;
        
        return *this;
    }
    
    static std::shared_ptr<Shader> MakeShader(GraphicsContext* _graphicsContext, const std::string& path, ShaderStage stage);
    
    /**
     * @brief - Compiles the current shader
     * @returns - returns if the shader was compiled
     */
    virtual bool Compile() = 0;

    std::size_t GetHash() {
        return hash_value(_path);
    };
    

    void DeclareShaderInput(ShaderInputParam param) {
        _shaderInputs.push_back(param);
    }

    /**
     * @brief Specifies a shader binding layout (only makes sense when using vertex shader)
     * @param binding - The binding point description
     * @param layouts - A list of shader layouts for the current binding
     */
    void DeclareShaderBindingLayout(ShaderInputBinding binding, const std::vector<ShaderInputLocation>& layouts) {
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
    
    void DeclareShaderOutput(const std::string& name) {};
    
    [[nodiscard]] inline const PushConstants& GetConstants() const {
        return _constants;
    };
    
    [[nodiscard]] inline const InputAttributes& GetInputAttributes() const {
        return _inputAttr;
    };
    
    [[nodiscard]] inline const ShaderStage GetShaderStage() const {
        return _stage;
    };
        
    GraphicsContext* _graphicsContext       = nullptr;
    
protected:
    ShaderStage _stage                  = ShaderStage::STAGE_UNDEFINED;
    std::string _path;
    PushConstants _constants;
    InputAttributes _inputAttr;
    ShaderInputs _shaderInputs;
};
