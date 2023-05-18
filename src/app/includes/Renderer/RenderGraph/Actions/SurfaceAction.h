#pragma once
#include "Renderer/Surface.h"
#include "Renderer/RenderPass/RenderPass.h"

struct SurfacePresentParams;
class Surface;

enum ESurfaceAction {
    SA_Empty,
    SA_Allocate,
    SA_Present
};

class SurfaceAction : public IGraphAction
{
public:
    bool Execute() override;

    SurfaceCreateParams _surfaceCreateParams;
    SurfacePresentParams _surfacePresentParams;
    std::shared_ptr<Surface> _surface;
    ESurfaceAction _surfaceAction;
};