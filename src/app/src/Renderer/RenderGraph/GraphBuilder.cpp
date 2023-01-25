#include "GraphBuilder.h"

GraphBuilder::GraphBuilder(RenderGraph* render_graph, std::string identifier)
    : render_graph_(render_graph)
    , graph_identifier_(identifier) {
}

// GraphBuilder::GraphBuilder(PersistentRenderTargets render_targets)
//     : persistent_render_targets_(render_targets) {
// }

VkCommandPool GraphBuilder::AllocateCommandPool() {
    // TODO this logic should be done by the render context
    RenderContext* render_context = render_graph_->GetRenderContext();
    if(!render_context) {
        return VK_NULL_HANDLE;
    }
    
    VkCommandPoolCreateInfo command_pool_create_info;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = render_context->GetGraphicsQueueIndex();

    VkCommandPool command_pool;
    const VkResult result = vkCreateCommandPool(render_context->GetLogicalDeviceHandle(), &command_pool_create_info, nullptr, &command_pool);

    return command_pool;
}