#include "Renderer/Vendor/Vulkan/VKWindow.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

void * VKWindow::CreateSurface(void *instance) const {
    VkSurfaceKHR surface;
    const VkResult result = glfwCreateWindowSurface(static_cast<VkInstance>(instance), window_, nullptr, &surface);

    if(result != VK_SUCCESS) {
        assert(0);
        return nullptr;
    }

    return surface;

}
