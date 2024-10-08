#include "Core/Scene.hpp"
#include "Core/Camera.hpp"
#include "Core/Light.hpp"

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
