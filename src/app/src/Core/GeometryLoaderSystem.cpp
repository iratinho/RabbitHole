#include "Core/GeometryLoaderSystem.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Core/Components/SceneComponent.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/RenderSystem.hpp"
#include "Renderer/RenderGraph/GraphBuilder.hpp"
#include "Renderer/RenderGraph/RenderGraph.hpp"

bool GeometryLoaderSystem::Initialize(InitializationParams initialization_params) {
    return true;
}

void GeometryLoaderSystem::Process(entt::registry& registry) {
    if(!_loadQueue.empty()) {
        std::string filePath = _loadQueue.front();
        _loadQueue.pop();

        // Load the actual geometry and store in the component
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate);

        // TODO Log Error
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "[Error]: Unable to load geometry file." << std::endl;
            return;
        }

        MeshNode meshNode;
        const std::function<void(aiNode*, MeshNode&)> processNode = [&scene, &processNode](const aiNode* node, MeshNode& meshNode)
        {
            /*
             * All primitives that belong to a mesh node are stored in the same GPU buffer
             * because of that we need to compute the data offset between primitives
             */
            int dataOffset = 0;
            
            for (unsigned int idx = 0; idx < node->mNumMeshes; ++idx)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[idx]];

                PrimitiveData primitive;
                primitive._primitiveName = mesh->mName.C_Str();

                for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData vertexData {};
                    vertexData.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
                    primitive._vertexData.push_back(vertexData);
                }

                for(unsigned int i = 0; i < mesh->mNumFaces; i++)
                {
                    aiFace face = mesh->mFaces[i];
                    for(unsigned int j = 0; j < face.mNumIndices; j++)
                        primitive._indices.push_back(face.mIndices[j]);
                }

                dataOffset += (primitive._indices.size() * sizeof(unsigned) + primitive._vertexData.size() * sizeof(float));
                
                meshNode._primitives.push_back(primitive);
            }

            // Recursively walk child nodes of this node
            for (int idx = 0; idx < node->mNumChildren; ++idx) {
                MeshNode newNode;
                newNode._nodeName = node->mChildren[idx]->mName.C_Str();
                processNode(node->mChildren[idx], newNode);
                meshNode._childNode.push_back(newNode);
            }
        };

        processNode(scene->mRootNode, meshNode);
        
        auto view = registry.view<SceneComponent>();
        
        for(auto entity: view) {
            SceneComponent& sceneComponent = registry.get<SceneComponent>(entity);
            sceneComponent._meshNodes.push_back(meshNode);
            break;
        }
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
