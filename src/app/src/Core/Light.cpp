#include "Core/Light.hpp"

Light::Light(const Light& light) {
    this->_scene = light._scene;
    this->_entity = light._entity;
}

Light::Light(Light&& light) {
    this->_scene = light._scene;
    this->_entity = light._entity;

    light._scene = nullptr;
}

Light::Light(Scene* scene, LightInitializationParams params) {
    _scene = scene;
    _entity = scene->GetRegistry().create();
    
    scene->GetRegistry().emplace<DirectionalLightComponent>(_entity, std::move(params.lightComponent));
    scene->AddObject(std::move(*this));
}
