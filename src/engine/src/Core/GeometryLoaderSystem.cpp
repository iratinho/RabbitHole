#include "Core/GeometryLoaderSystem.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Core/Components/MeshComponent.hpp"
#include "Core/MeshObject.hpp"
#include "Core/Scene.hpp"
#include "Renderer/Buffer.hpp"

bool GeometryLoaderSystem::Initialize(InitializationParams initialization_params) {
    return true;
}

void GeometryLoaderSystem::Process(Scene* const scene) {
    if(!_loadQueue.empty()) {
        std::string filePath = _loadQueue.front();
        _loadQueue.pop();

        MeshInitializationParams meshParams;
        meshParams._filePath = std::move(filePath);
        const Mesh& mesh = MeshFactory::MakeObject(scene, meshParams);
    }
}

void GeometryLoaderSystem::EnqueueFileLoad(const std::string filePath) {
    _loadQueue.push(filePath);
}

void GeometryLoaderSystem::ForEachNodeConst(const MeshNode* meshNode, std::function<void(const MeshNode*)> func) {
    if(!meshNode) {
        return;
    }

    func(meshNode);
    
    for (const MeshNode& node : meshNode->_childNode) {
        func(&node);
        ForEachNodeConst(&node, func);
    }    
}

void GeometryLoaderSystem::ForEachNode(MeshNode* meshNode, std::function<void(MeshNode*)> func) {
    if(!meshNode) {
        return;
    }
    func(meshNode);
    
    for (MeshNode& node : meshNode->_childNode) {
        func(&node);
        ForEachNode(&node, func);
    }    
}
