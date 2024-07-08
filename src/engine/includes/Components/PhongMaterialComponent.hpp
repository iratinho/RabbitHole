#pragma once 

#include "Common.hpp"
#include "MaterialComponent.hpp"

class PhongMaterialComponent : public MaterialComponent {
public:
    PhongMaterialComponent() : MaterialComponent() {
        _fragShaderPath = "shaders/floor_grid.frag.spv";
        _vertexShaderPath = "shaders/floor_grid.vert.spv";
    }
};
