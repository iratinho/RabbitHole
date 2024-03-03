#pragma once
#include "Core/IBaseObject.hpp"

class ITextureInterface;

class IMaterialInterface {};

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_BYTE_CODE_DIR) "/" STR(name)

/// Material class for matcap shader
class MatCapMaterial : public IMaterialInterface {
public:
    MatCapMaterial() = default;
    MatCapMaterial(std::unique_ptr<ITextureInterface> sphericalTexture);
    virtual ~MatCapMaterial();
    
    /// Returns the spherical texture map used for the matcap shader
    inline std::shared_ptr<ITextureInterface> GetMatCapTexture() { return _texture; };
    
private:
    std::shared_ptr<ITextureInterface> _texture;
};


class Material {
public:
    virtual const char* GetFragmentShaderPath() = 0;
    virtual const char* GetVertexShaderPath() = 0;
};

class FloorGridMaterial : public Material {
public:
    const char *GetFragmentShaderPath() override { return COMBINE_SHADER_DIR(floor_grid.frag); };
    const char *GetVertexShaderPath() override { return COMBINE_SHADER_DIR(floor_grid.vert); };
};
