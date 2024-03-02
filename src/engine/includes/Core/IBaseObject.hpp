#pragma once

#include "entt/fwd.hpp"

class Scene;

template <typename T>
class IBaseObjectFactory {
public:
    ~IBaseObjectFactory() {};
    
    template <typename ...Params>
    static T MakeObject(Params&&... params) {
        return T(std::forward<Params>(params)...);
    };
};

/// Base class for objects backed by the entity system
class IBaseObject {
public:
    IBaseObject() = default;
    virtual const char* GetIdentifier() {}; // virtual just to trigger compile time error
    entt::entity GetEntity() const { return _entity; };
        
protected:
    entt::entity _entity;
    Scene* _scene = nullptr;
};
