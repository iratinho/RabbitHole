#pragma once
#include <entt/entity/registry.hpp>
#include <queue>
#include "Renderer/render_context.h"

struct MeshNode;

class GeometryLoaderSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    void Process(entt::registry& registry);
    void EnqueueFileLoad(const std::string filePath);

    static void ForEachNodeConst(const MeshNode* meshNode, std::function<void(const MeshNode*)> func);
    static void ForEachNode(MeshNode* meshNode, std::function<void(MeshNode*)> func);

private:
    std::queue<std::string> _loadQueue;
};
