#include "Core/MeshObject.hpp"
#include "Core/Scene.hpp"
#include "Core/Components/TransformComponent.hpp"
#include "Core/Components/MeshComponent.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

Mesh::Mesh(Scene* const scene, MeshInitializationParams params) {
    _scene = scene;
    _entity = scene->GetRegistry().create();

    if(!params._filePath.empty()) {
        BuildFromFile(params._filePath.c_str());
    }
    
    TransformComponent transformComponent {};
    transformComponent.m_Position = params._position;
    scene->GetRegistry().emplace<TransformComponent>(_entity, transformComponent);

    scene->AddObject(std::move(*this));
}

Mesh::Mesh(const Mesh& mesh) {
    _scene = mesh._scene;
    _entity = mesh._entity;
}

Mesh::Mesh(Mesh&& mesh) {
    _scene = mesh._scene;
    _entity = mesh._entity;

    mesh._scene = nullptr;
}

void Mesh::BuildFromFile(const char* file) {
    // Load the actual geometry and store in the component
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);

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
        unsigned int indicesOffset = 0;
        unsigned int vertexOffset = 0;

        for (int idx = 0; idx < node->mNumMeshes; ++idx)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[idx]];

            PrimitiveData primitive;
            primitive._primitiveName = mesh->mName.C_Str();

            for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
                VertexData vertexData {};
                vertexData.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
                vertexData.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
                primitive._vertexData.push_back(vertexData);
            }

            for(unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for(unsigned int j = 0; j < face.mNumIndices; j++)
                    primitive._indices.push_back(face.mIndices[j]);
            }

            unsigned int currentVertexOffset = primitive._indices.size() * sizeof(unsigned int)
                                               + indicesOffset  + vertexOffset;

            primitive._indicesOffset = indicesOffset;
            primitive._vertexOffset = currentVertexOffset;

            meshNode._primitives.push_back(primitive);

            indicesOffset += (primitive._indices.size() * sizeof(unsigned)) * std::clamp(idx, 0, 1);
            vertexOffset += (primitive._vertexData.size() * sizeof(VertexData)) * std::clamp(idx, 0, 1);
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

    MeshComponent meshComponent;
    meshComponent._meshNodes.push_back(meshNode);
    _scene->GetRegistry().emplace<MeshComponent>(_entity, meshComponent);
}

void Mesh::ForEachNode(std::function<void(const MeshNode*)> func) const {
    MeshComponent& rootComponent = _scene->GetComponents<MeshComponent>(_entity);
    
    for(const auto& rootNode : rootComponent._meshNodes) {
        func(&rootNode);
        
        for(const MeshNode& childNode : rootNode._childNode) {
            func(&childNode);
        }
    }
}

