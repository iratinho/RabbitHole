#pragma once

class RenderTarget;

struct UserInterfaceComponent {
    std::shared_ptr<RenderTarget> _uiRenderTarget;
};