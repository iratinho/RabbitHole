#pragma once
#include "entt/fwd.hpp"

using ComponentID = entt::entity;

class ComponentIDCounter {
public:
    static int Increment() {
        static ComponentIDCounter counter;
        counter._id = ++counter._id;
        
        return counter._id;
    };
    
private:
    int _id = -1;
};


class CommonComponent {
public:
    // Auto increments component id
    CommonComponent() {
        _id = ComponentIDCounter::Increment();
    }

    uint32_t _id;
};

#define DECLARE_CONSTRUCTOR(type, parent) \
    type() : parent() {}
