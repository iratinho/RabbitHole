#include "Renderer/Event.hpp"

#ifdef VULKAN_BACKEND
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#endif

std::unique_ptr<Event> Event::MakeEvent(const InitializationParams& params) {
    std::unique_ptr<Event> instance;

#ifdef VULKAN_BACKEND
    instance = std::make_unique<VKEvent>();
    instance->_params = params;
#endif
    
    if(instance) {
        instance->Initialize();
    }

    return instance;
};
