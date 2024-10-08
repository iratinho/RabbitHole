#pragma once
#include "entt/entt.hpp"
#include "Core/IBaseObject.hpp"
#include "GenericInstanceWrapper.hpp"

class Camera;
class Mesh;
class Light;

/// Scene class holds information about objects used in a renderable world. Ex: meshes, cameras, etc..
class Scene {
public:
    Scene() = default;

    template <typename T>
    void AddObject(T&& object) {
        _wrappedObjects.push_back(std::forward<T>(object));
    };

    /// Sets a camera as active, meaning that we will render the scene based on this camera view
    /// - Parameter camera: camera to represent the rendering view
    void SetActiveCamera(const Camera& camera);
    
    /// Returns the current active camera for the scene
    /// - Parameter camera: a ref for the current active camera used o represent the rendering view
    bool GetActiveCamera(Camera& camera);
    
    Light* GetLight();

    inline entt::registry& GetRegistry() { return _registry; };
    
    // Deprecate
    template <typename ...Components>
    decltype(auto) GetComponents(entt::entity entity) {
        auto view =  GetView<Components...>();
        return view.template get<Components...>(entity);
    }
    
    template<typename ComponentsTuple>
    auto GetRegistryView() {
        return getViewHelper<ComponentsTuple>(std::make_index_sequence<std::tuple_size_v<ComponentsTuple>>{});
    }
        
private:
    template<typename Tuple, std::size_t... Is>
    auto getViewHelper(std::index_sequence<Is...>) {
        return GetRegistry().view<std::tuple_element_t<Is, Tuple>...>();
    }
    
    // Deprecate
    template <typename ...Components>
    decltype(auto) GetView() {
        return GetRegistry().view<Components...>();
    }
    
private:
    std::vector<GenericInstanceWrapper<IBaseObject>> _wrappedObjects;
    entt::entity _activeCamera;
    entt::registry _registry;
};
