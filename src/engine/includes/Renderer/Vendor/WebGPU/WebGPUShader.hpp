#pragma once
#include "Renderer/Shader.hpp"
#include "webgpu/webgpu.hpp"

class WGPUVertexState;

class WebGPUShader : public Shader {
public:
    using Shader::Shader;

    bool Compile() override;
    
    [[nodiscard]] WGPUShaderModule GetWebGPUShaderModule() const {
        return _module;
    }
    
private:
    WGPUShaderModule _module;
};
