#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/MeshObject.hpp"
#include "Core/Light.hpp"
#include "Core/Components/MeshComponent.hpp"


void Scene::SetActiveCamera(const Camera& camera) {
    _activeCamera = camera.GetEntity();
}

bool Scene::GetActiveCamera(Camera& camera) {
    for (auto& object : _wrappedObjects) {
        if(object->GetEntity() == _activeCamera) {
            camera = (Camera&)object.GetRef();
            return true;
        }
    }

    return false;
}

Light* Scene::GetLight() {
    for (auto& object : _wrappedObjects) {
        if(Light* light = dynamic_cast<Light*>(object.operator->())) {
            return light;
        }
    }
    
    return nullptr;
}

void Scene::ForEachMesh(std::function<void(Mesh*)> func) {
    const auto view = GetView<const MeshComponent>();

    // TODO should we register a Mesh and group it with MeshComponent in entt?
    // https://skypjack.github.io/2019-04-12-entt-tips-and-tricks-part-1/
    // TODO this still looks strange, rethink approach
    // We can have a lookup map to allow us fast access to entity object (mesh, light, etc...)
    for (auto& object : _wrappedObjects) {
        if(Mesh* mesh = dynamic_cast<Mesh*>(object.operator->())) {
            func(mesh);
        }
    }
}

entt::registry& Scene::GetRegistry() {
    return _registry;
}

