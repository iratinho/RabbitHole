#include "Renderer/Vendor/Vulkan/VKWindow.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

void * VKWindow::CreateSurface(void *instance) {
    VkSurfaceKHR surface;
    const VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), _window, nullptr, &surface);

    if(result != VK_SUCCESS) {
        assert(0);
        return nullptr;
    }

    return surface;

}
