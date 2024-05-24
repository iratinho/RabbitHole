#pragma once
#include "entt/entt.hpp"
#include "Core/IBaseObject.hpp"
#include "GenericInstanceWrapper.hpp"

class Camera;
class Mesh;
class Light;
class MatCapMaterial;

/// Scene class holds information about objects used in a renderable world. Ex: meshes, cameras, etc..
class Scene {
public:
    Scene();
    
    template <typename T>
    void AddObject(T&& object) {
        _wrappedObjects.push_back(std::forward<T>(object));
    };
    
    /// Toggles the matcap mode visualization for entire scene
    void ToggleMatCapMode();
    
    /// Whether  matpcap rendering mode is enabled or not
    /// - Returns true if the matcap rendering mode is enabled
    inline bool IsMatpCapModeEnabled() { return _bIsMatCapEnabled; };
    
    /// Sets a new matcap material to be used when rendering with matcap mode enabled
    void SetMatCapMaterial(std::unique_ptr<MatCapMaterial> matCapMaterial);
    
    /// Gets the current matcap material to be used when rendering with matcap mode enabled
    ///  - Returns matcap material
    inline std::shared_ptr<MatCapMaterial> GetMatCapMaterial() { return _matCapMaterial; }
    
    /// Sets a camera as active, meaning that we will render the scene based on this camera view
    /// - Parameter camera: camera to represent the rendering view
    void SetActiveCamera(const Camera& camera);
    
    /// Returns the current active camera for the scene
    /// - Parameter camera: a ref for the current active camera used o represent the rendering view
    bool GetActiveCamera(Camera& camera);
    
    Light* GetLight();
    
    void ForEachMesh(std::function<void(Mesh*)> func);

    inline entt::registry& GetRegistry() { return _registry; };
    
    // Deprecate
    template <typename ...Components>
    decltype(auto) GetComponents(entt::entity entity) {
        auto view =  GetView<Components...>();
        return view.template get<Components...>(entity);
    }
    
    template<typename ComponentsTuple>
    const auto GetRegistryView() {
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
    
    bool _bIsMatCapEnabled;
    std::shared_ptr<MatCapMaterial> _matCapMaterial;
};
