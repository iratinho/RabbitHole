#pragma once
#include "Renderer/GPUDefinitions.h"

class ShaderCompiler {
public:
    static ShaderCompiler& Get();
    static std::vector<char> CompileStatic(const char* path, ShaderStage shaderStage);
    std::vector<unsigned int> Compile(const char* path, ShaderStage shaderStage);
};
