#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"
#include "Renderer/Surface.hpp"

struct SurfaceactionActionData {
    std::shared_ptr<Surface> _surface;
};

struct SurfaceAllocateActionData : public SurfaceactionActionData {
    SurfaceCreateParams _surfaceCreateParams;
};

struct SurfacePresentActionData : public SurfaceactionActionData {
    SurfacePresentParams _surfacePresentParams;
};

class SurfaceAction : public IGraphAction {
public:
    SurfaceAction() = delete;
    SurfaceAction(const std::any& actionData);
    bool Execute() override;
};
