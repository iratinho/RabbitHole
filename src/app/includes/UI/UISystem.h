#pragma once
#if !defined(__APPLE__)
#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>
#endif

#include "entt/entity/registry.hpp"

namespace app
{
    namespace window
    {
        class Window;
    }
}

class RenderTarget;
class RenderContext;

struct InitializationParams;

class UISystem {
public:
    bool Initialize(RenderContext* renderContext, InitializationParams initialization_params);
    bool Process(entt::registry& registry);

private:
    void PumpMessages() const;
    
    RenderContext* _renderContext;
    app::window::Window* _window;
    std::shared_ptr<RenderTarget> _uiRenderTarget;
    
#if !defined(__APPLE_CC__)
    ultralight::RefPtr<ultralight::Renderer> _renderer;
    ultralight::RefPtr<ultralight::View> _view;
#endif
};
