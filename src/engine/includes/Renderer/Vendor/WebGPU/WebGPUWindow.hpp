#pragma once
#include "window.hpp"

class WebGPUWindow : public Window {
public:
    void * CreateSurface(void *instance) const override { Window::CreateSurface(instance); return instance; };
};