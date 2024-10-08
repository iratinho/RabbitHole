#pragma once
#include <entt/entity/registry.hpp>

struct MeshNode;
class Scene;

class GeometryLoaderSystem {
public:
    void Process(Scene* scene);
    void EnqueueFileLoad(const std::string& filePath);

private:
    static void LoadFromFile(Scene* scene, const std::string& filePath);
    
    std::queue<std::string> _loadQueue;
};
