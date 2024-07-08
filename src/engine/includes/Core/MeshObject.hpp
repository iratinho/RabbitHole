#pragma once
#include "Core/IBaseObject.hpp"
#include "Components/MeshComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "glm/glm.hpp"
#include "Scene.hpp"

struct PrimitiveProxy;
struct MeshNode;

struct MeshInitializationParams {
    glm::vec3 _position = {};
    std::string _filePath;
};

/*
* Mesh is the parent object, it contains a meshcomponent where its specified all the childs
* This mesh object will have a transform that should propagate to childs
*/
class Mesh : public IBaseObject {
private:
    Mesh(Scene* const scene, MeshInitializationParams params);
    friend class IBaseObjectFactory<Mesh>;

public:
    Mesh() = default;
    Mesh(const Mesh& mesh);
    
    void operator=(const Mesh& mesh) {
        this->_scene = mesh._scene;
        this->_entity = mesh._entity;
    }
    
    template <typename ...Components>
    std::tuple<Components&...> GetComponents() {
        return _scene->GetComponents<Components...>(_entity);
    }

    decltype(auto) GetComponents() const  {
        return _scene->GetComponents<const MeshComponent, const TransformComponent>(_entity);
    };
    
    void ForEachNode(std::function<void(MeshNode*)> func) const;
    
    void ComputeMatrix();
    
    void CreatePrimitive(std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec3> tangents);
    
    void AddMeshNode(const MeshNode& meshNode);
    
private:
    void BuildFromFile(const char* file);
};

class MeshFactory : public IBaseObjectFactory<Mesh> { };
