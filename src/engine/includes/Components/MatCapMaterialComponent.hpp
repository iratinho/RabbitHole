#pragma once
#include "Components/MaterialComponent.hpp"

class Texture2D;

class MatCapMaterialComponent : public MaterialComponent {
public:
    MatCapMaterialComponent() : MaterialComponent() {
        _fragShaderPath = "shaders/matcap.frag.spv";
        _vertexShaderPath = "shaders/matcap.vert.spv";
    }

    std::shared_ptr<Texture2D> _matCapTexture; 
};

// Sotring Textures with shared ptr, i know its wierd to have pointers inside components because of locality but this is a tradeoff perf/memory wise
// this kinda matters when we have a lot of textures and we want to share them between components
