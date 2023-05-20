#include "Renderer/RenderGraph/Actions/SurfaceAction.hpp"
#include "Renderer/Surface.hpp"

bool SurfaceAction::Execute()
{
    if(!_surface) {
        return false;
    }

    switch (_surfaceAction) {
        case SA_Empty: return false;
        case SA_Allocate: _surface->AllocateSurface(_surfaceCreateParams); break;
        case SA_Present: _surface->Present(_surfacePresentParams); break;
    }

    return true;
}
