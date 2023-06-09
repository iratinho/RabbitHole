#pragma once
#include "GPUDefinitions.h"

class ShaderCompiler {
public:
    static ShaderCompiler* Get();
    std::vector<unsigned int> Compile(const char* path, ShaderStage shaderStage);
};
