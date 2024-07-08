#pragma once
#include "Core/IBaseObject.hpp"
#include "Core/Scene.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/DirectionalLightComponent.hpp"
#include "Components/TransformComponent.hpp"

struct LightInitializationParams {
    DirectionalLightComponent lightComponent;
};

class Light : public IBaseObject {
    Light(Scene* scene, LightInitializationParams params);
    friend class IBaseObjectFactory<Light>;

public:
    Light() = default;
    Light(const Light& light);
    Light(Light&& light);

    void operator=(const Light& light) {
        this->_scene = light._scene;
        this->_entity = light._entity;
    }

    void operator=(Light&& light) {
        this->_scene = light._scene;
        this->_entity = light._entity;

        light._scene = nullptr;
    }

    decltype(auto) GetComponents() {
        return _scene->GetComponents<DirectionalLightComponent>(_entity);
    };
};

class LightFactory : public IBaseObjectFactory<Light> { };
