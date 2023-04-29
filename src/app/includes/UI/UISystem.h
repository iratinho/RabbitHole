#pragma once
#include <Ultralight/Renderer.h>
#include <Ultralight/View.h>

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
    ultralight::RefPtr<ultralight::Renderer> _renderer;
    ultralight::RefPtr<ultralight::View> _view;
    std::shared_ptr<RenderTarget> _uiRenderTarget;
};
