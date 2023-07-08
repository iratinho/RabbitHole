#include "Core/Scene.hpp"
#include "Core/Camera.hpp"

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

entt::registry& Scene::GetRegistry() {
    return _registry;
}

