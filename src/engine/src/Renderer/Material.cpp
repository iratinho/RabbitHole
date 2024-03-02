#include "Renderer/Material.hpp"
#include "Renderer/Interfaces/TextureInterface.hpp"

MatCapMaterial::MatCapMaterial(std::unique_ptr<ITextureInterface> sphericalTexture) {
    _texture = std::move(sphericalTexture);
}

MatCapMaterial::~MatCapMaterial() {};
