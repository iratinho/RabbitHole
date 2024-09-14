#pragma once
#include "Renderer/Device.hpp"

class VKDevice : public Device {
public:
    bool Initialize() override;
    void Shutdown() override;
};
