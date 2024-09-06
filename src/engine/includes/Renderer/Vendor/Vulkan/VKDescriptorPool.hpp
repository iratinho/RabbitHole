#pragma once
#include "Core/Containers/ObjectPool.hpp"
#include "vulkan/vulkan.hpp"

class GraphicsContext;

class VKDescriptorPool {
public:
    VKDescriptorPool() = default;
    
    VKDescriptorPool(VkDescriptorPool descriptorPool, VkDescriptorSetLayout parentLayout)
        : _descriptorPool(descriptorPool)
        , _parentLayout(parentLayout) {
    };
    
    VkDescriptorSet AcquireDescriptorSet(GraphicsContext* graphicsContext);
    
private:
    void AllocateDescriptors(GraphicsContext* graphicsContext);
    
private:
    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _parentLayout;
    ObjectPool<VkDescriptorSet> _pool;
};
