#pragma once

#include "Common.hpp"

class PrimitiveComponent : public CommonComponent {
public:
    DECLARE_CONSTRUCTOR(PrimitiveComponent, CommonComponent)
    std::string _identifier;
    ComponentID _materialEntity;
    ComponentID _transformEntity;
    ComponentID _proxyEntity; // CPU data proxy
};
