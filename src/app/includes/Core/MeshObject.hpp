#pragma once
#include "Core/IBaseObject.hpp"

struct MeshInitializationParams {
};

/*
* Mesh is the parent object, it contains a meshcomponent where its specified all the childs
* This mesh object will have a transform that should propagate to childs
*/
class Mesh : public IBaseObject {
private:
    Mesh(Scene* scene, MeshInitializationParams params);
    friend class IBaseObjectFactory<Mesh>;

public:
    Mesh() = default;
};

template <class T>
class MeshFactory : public IBaseObjectFactory<T> { };
