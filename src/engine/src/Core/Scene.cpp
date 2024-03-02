#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/MeshObject.hpp"
#include "Core/Light.hpp"
#include "Core/Components/MeshComponent.hpp"
#include "Renderer/Material.hpp"

Scene::Scene()
    : _bIsMatCapEnabled(false){
    
}

void Scene::ToggleMatCapMode() {
    _bIsMatCapEnabled = !_bIsMatCapEnabled;
}

void Scene::SetMatCapMaterial(std::unique_ptr<MatCapMaterial> matCapMaterial) {
    _matCapMaterial.reset();
    _matCapMaterial = std::move(matCapMaterial);
}

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

    // Make mesh object factory constructible with scene and entity id
    // this way we can iterate over all mesh components in the view and construct the mesh object
    // same goes for light and cameras
    
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
