#pragma once
#include "Window/Desktop/DesktopWindow.hpp"

class VKWindow : public DesktopWindow {
public:
    void * CreateSurface(void *instance) override;
};

