#pragma once
#include "Renderer/Texture2D.hpp"
#include "vulkan/vulkan.hpp"

class Range;

class VkTexture2D : public Texture2D {
public:
    VkTexture2D() = default;

    VkSampler MakeSampler();
                    
private:
    friend class VkTextureResource;
};

