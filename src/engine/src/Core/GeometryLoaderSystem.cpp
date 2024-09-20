#include "Core/GeometryLoaderSystem.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Components/MeshComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/MatCapMaterialComponent.hpp"
#include "Components/PrimitiveProxyComponent.hpp"
#include "Core/MeshObject.hpp"
#include "Core/Scene.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Texture2D.hpp"

bool GeometryLoaderSystem::Initialize() {
    return true;
}

void GeometryLoaderSystem::Process(Scene* scene) {
    if(!_loadQueue.empty()) {
        std::string filePath = _loadQueue.front();
        _loadQueue.pop();

//        MeshInitializationParams meshParams;
//        meshParams._filePath = std::move(filePath);
//        const Mesh& mesh = MeshFactory::MakeObject(scene, meshParams);
        LoadFromFile(scene, filePath);
    }
}

void GeometryLoaderSystem::EnqueueFileLoad(const std::string filePath) {
    _loadQueue.push(filePath);
}

void GeometryLoaderSystem::LoadFromFile(Scene* scene, const std::string filePath) {
    Assimp::Importer importer;
    const aiScene* aiScene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_ValidateDataStructure);
    float unitScaleFactor = 0.0;
//    bool failedScaleProp = aiScene->mMetaData->Get("UnitScaleFactor", unitScaleFactor);
    
    float factor = (1.f / 100.f) * unitScaleFactor;
    glm::mat4 unitMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(factor));
    
    // Create a texture2D with the matcap material we want to use, this is a temporary solution until we have proper UI to assign stuff
    auto matCapTexture = Texture2D::MakeFromPath("matcaps/matcap.jpg", Format::FORMAT_R8G8B8A8_SRGB);

    MeshComponentNew meshGroup;

    const std::function<void(aiNode*, TransformComponent*)> processNode = [&aiScene, scene, &processNode, &meshGroup, &matCapTexture](const aiNode* node, TransformComponent* parentTransform) {
        TransformComponent nodeTransform;
        std::memcpy(&nodeTransform._matrix[0], &node->mTransformation.a1, sizeof(aiMatrix4x4));
        nodeTransform._matrix = glm::transpose(nodeTransform._matrix);
        nodeTransform._isRootTransform = parentTransform == nullptr;
        
        if(parentTransform) {
            parentTransform->_childs.push_back(nodeTransform._id);
        }

        // Process child meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = aiScene->mMeshes[node->mMeshes[i]];
            PrimitiveProxyComponentCPU primitiveComponent;
            MatCapMaterialComponent materialComponent;
            materialComponent._matCapTexture = matCapTexture;
            
            // Extract vertex data
            for(unsigned int x = 0; x < mesh->mNumVertices; x++) {
                VertexData vertexData {};
                vertexData.position = {mesh->mVertices[x].x, mesh->mVertices[x].y, mesh->mVertices[x].z};
                vertexData.normal = {mesh->mNormals[x].x, mesh->mNormals[x].y, mesh->mNormals[x].z};
            
                primitiveComponent._vertexData.push_back(vertexData);
            }
            
            // Extract indices
            for (unsigned int x = 0; x < mesh->mNumFaces; x++) {
                auto face = mesh->mFaces[x];
                for(unsigned int j = 0; j < face.mNumIndices; j++) {
                    primitiveComponent._indices.push_back(face.mIndices[j]);
                }
            }
            
            TransformComponent primitiveTransform;
            primitiveTransform._matrix = glm::identity<glm::mat4>();
            nodeTransform._childs.push_back(primitiveTransform._id);
            
            entt::entity primitiveEntity = scene->GetRegistry().create();
            scene->GetRegistry().emplace<PrimitiveProxyComponentCPU>(primitiveEntity, primitiveComponent);
            scene->GetRegistry().emplace<MatCapMaterialComponent>(primitiveEntity, materialComponent);
            scene->GetRegistry().emplace<TransformComponent>(primitiveEntity, primitiveTransform);
            
            meshGroup._primitives.push_back(primitiveEntity);
        }
        
        // Process child nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            aiNode* childNode = node->mChildren[i];
            processNode(childNode, &nodeTransform);
        }
        
        entt::entity nodeEntity = scene->GetRegistry().create();
        scene->GetRegistry().emplace<TransformComponent>(nodeEntity, nodeTransform);
    };
    
    processNode(aiScene->mRootNode, nullptr);
    
    entt::entity meshGroupEntity = scene->GetRegistry().create();
    scene->GetRegistry().emplace<MeshComponentNew>(meshGroupEntity, meshGroup);
    
}

