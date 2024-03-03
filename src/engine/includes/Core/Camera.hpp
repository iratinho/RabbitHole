#pragma once
#include "glm/glm.hpp"
#include "entt/entt.hpp"
#include "Core/IBaseObject.hpp"
#include "Core/Scene.hpp"
#include "Components/InputComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/TransformComponent.hpp"

class CameraComponent;

struct CameraInitializationParams {
    glm::vec3 _position;
    float _fov;
};

class Camera : public IBaseObject {
private:
    Camera(Scene* scene, CameraInitializationParams params);
    friend class IBaseObjectFactory<Camera>;

public:
    Camera() = default;
    Camera(const Camera& camera);
    Camera(Camera&& camera);

    void operator=(const Camera& camera) {
        this->_scene = camera._scene;
        this->_entity = camera._entity;
    }

    void operator=(Camera&& camera) {
        this->_scene = camera._scene;
        this->_entity = camera._entity;

        camera._scene = nullptr;
    }
    
    static constexpr char* IDENTIFIER = "CAMERA";
    const char * GetIdentifier() override;

    decltype(auto) GetComponents() {
        return _scene->GetComponents<InputComponent, CameraComponent, TransformComponent>(_entity);
    };
            
    float GetFOV();
    glm::vec3 GetPosition();
    glm::mat4 GetViewMatrix();
};

class CameraFactory : public IBaseObjectFactory<Camera> { };

