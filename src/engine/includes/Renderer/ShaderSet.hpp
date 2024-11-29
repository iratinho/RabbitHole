#pragma once
#include "Renderer/Shader.hpp"

class RenderPassInterface;
class GraphicsPipeline;

// Global instance shared with both vertex and fragment shader
class ShaderSet {
public:
    ShaderSet(RenderPassInterface* renderPass);
    
    bool CompileShaders();
    
    [[nodiscard]] const Shader* GetVertexShader() const {
        return _vertexShader.get();
    }
    
    [[nodiscard]] const Shader* GetFragmentShader() const {
        return _fragmentShader.get();
    }
    
    // TODO should i keep all of this data here, or should i pass the render pass pointer into the shaders?
    
    [[nodiscard]] const ShaderInputBindings& GetInputBindings() const {
        return _inputBindings;
    }
    
    [[nodiscard]] const std::vector<ShaderDataStream>& GetDataStreams() const {
        return _dataStreams;
    }
    
private:
    std::unique_ptr<Shader> _vertexShader;
    std::unique_ptr<Shader> _fragmentShader;
    
    ShaderInputBindings _inputBindings; // Use in vertex shader
    std::vector<ShaderDataStream> _dataStreams; // Used in both vertex and fragment shaders
    
    RenderPassInterface* _renderPass = nullptr;
};
