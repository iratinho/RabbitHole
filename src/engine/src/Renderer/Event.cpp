#include "Renderer/Event.hpp"

#ifdef USING_VULKAN_API
#include "Renderer/Vendor/Vulkan/VKEvent.hpp"
#endif

std::unique_ptr<Event> Event::MakeEvent(const InitializationParams& params) {
    std::unique_ptr<Event> instance;

#ifdef USING_VULKAN_API
    instance = std::make_unique<VKEvent>();
    instance->_params = params;
#endif
    
    if(instance) {
        instance->Initialize();
    }

    return instance;
};
