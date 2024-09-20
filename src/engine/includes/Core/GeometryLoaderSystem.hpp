#pragma once
#include <entt/entity/registry.hpp>

struct MeshNode;
class Scene;

class GeometryLoaderSystem {
public:
    bool Initialize();
    void Process(Scene* scene);
    void EnqueueFileLoad(const std::string filePath);

private:
    
    void LoadFromFile(Scene* scene, const std::string filePath);
    
    std::queue<std::string> _loadQueue;
};
