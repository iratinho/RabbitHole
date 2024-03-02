#pragma once
#include "Renderer/Texture2D.hpp"

class Range;

class VkTexture2D : public Texture2D {
public:
    VkTexture2D() = default;
                    
private:
    friend class VkTextureResource;
};

