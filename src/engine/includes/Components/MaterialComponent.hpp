#pragma once
#include "Common.hpp"

class MaterialComponent : public CommonComponent {
public:
    DECLARE_CONSTRUCTOR(MaterialComponent, CommonComponent)
    std::string _identifier;

protected:
    std::string _fragShaderPath;
    std::string _vertexShaderPath;
};
