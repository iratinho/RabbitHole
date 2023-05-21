#include "Renderer/RenderGraph/Actions/SurfaceAction.hpp"
#include "Renderer/Surface.hpp"

SurfaceAction::SurfaceAction(const std::any& actionData) {
    IGraphAction::_actionData = actionData;
};

bool SurfaceAction::Execute() {
    // Allocate
    if(SurfaceAllocateActionData* data = std::any_cast<SurfaceAllocateActionData>(&_actionData)) {
        if(data->_surface) {
            data->_surface->AllocateSurface(data->_surfaceCreateParams);
            return true;
        }
    }
    
    // Present
    if(SurfacePresentActionData * data = std::any_cast<SurfacePresentActionData>(&_actionData)) {
        if(data->_surface) {
            data->_surface->Present(data->_surfacePresentParams);
            return true;
        }
    }
   
    return false;
}
