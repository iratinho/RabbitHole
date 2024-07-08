#pragma once 

#include "Common.hpp"
#include "MaterialComponent.hpp"

class GridMaterialComponent : public MaterialComponent {
public:
    GridMaterialComponent() : MaterialComponent() {
        _fragShaderPath = "shaders/floor_grid.frag.spv";
        _vertexShaderPath = "shaders/floor_grid.vert.spv";
    }
};
