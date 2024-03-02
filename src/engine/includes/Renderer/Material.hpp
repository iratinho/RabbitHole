#pragma once
#include "Core/IBaseObject.hpp"

class ITextureInterface;

class IMaterialInterface {};

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
