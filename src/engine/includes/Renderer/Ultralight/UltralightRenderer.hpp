#pragma once
#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>

#include "entt/entity/registry.hpp"

class Texture2D;
class Window;
class Device;
struct InitializationParams;

class UltralightRenderer {
public:
    bool Initialize(Device* device);
    bool Render();
    const void* LockPixels() const;
    void UnlockPixels() const;
    std::size_t PixelBufferSize() const;

private:
    void PumpMessages() const;
    
    Device* _device = nullptr;
    Window* _window = nullptr;
    
   ultralight::RefPtr<ultralight::Renderer> _renderer;
   ultralight::RefPtr<ultralight::View> _view;

    bool _bIsDirty = true;
};
