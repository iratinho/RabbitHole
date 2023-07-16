#pragma once
#include "Core/IBaseObject.hpp"
#include "glm.hpp"

struct PrimitiveProxy;
struct MeshNode;

struct MeshInitializationParams {
    glm::vec3 _position;
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
    Mesh(Mesh&& mesh);
    
    void operator=(const Mesh& mesh) {
        this->_scene = mesh._scene;
        this->_entity = mesh._entity;
    }

    void operator=(Mesh&& mesh) {
        this->_scene = mesh._scene;
        this->_entity = mesh._entity;

        mesh._scene = nullptr;
    }
    
    void ForEachNode(std::function<void(const MeshNode*)> func) const;
    
private:
    void BuildFromFile(const char* file);
};

class MeshFactory : public IBaseObjectFactory<Mesh> { };
