#include "render_context.h"
#include "OpaqueRenderer.h"

#include <RenderSystem.h>
#include <RenderTarget.h>
#include <Texture.h>

#include "window.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"

bool OpaqueRenderer::Initialize(RenderContext* const render_context, const InitializationParams& initialization_params)
{
    if (render_context)
    {
        render_context_ = render_context;

        VALIDATE_RETURN(CreateRenderPass());
        VALIDATE_RETURN(CreateGraphicsPipeline());
        // VALIDATE_RETURN(CreateFrameBuffers());

        window_ = initialization_params.window_;
    }

    return false;
}

bool OpaqueRenderer::AllocateCommandBuffers(VkCommandPool command_pool, int pool_idx)
{
    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo command_buffer_allocate_info;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.pNext = nullptr;
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandBufferCount = 1;

    const VkResult result = vkAllocateCommandBuffers(render_context_->GetLogicalDeviceHandle(),
                                                     &command_buffer_allocate_info, &command_buffer);
    if (result != VK_SUCCESS)
    {
        return false;
    }

    command_buffers_.push_back(command_buffer);

    return true;
}

bool OpaqueRenderer::AllocateFrameBuffers(int idx, PresistentRenderTargets render_targets)
{
    // We should get this data from the render targets
    auto extent = render_context_->GetSwapchainExtent();

    const std::vector<VkImageView> image_views = {
        render_targets.scene_color_render_target->GetRenderTargetView(), render_targets.scene_depth_render_target->GetRenderTargetView()
    };
    
    VkFramebufferCreateInfo framebuffer_create_info;
    framebuffer_create_info.flags = 0;
    framebuffer_create_info.height = extent.height;
    framebuffer_create_info.layers = 1;
    framebuffer_create_info.width = extent.width;
    framebuffer_create_info.attachmentCount = image_views.size();
    framebuffer_create_info.pAttachments = image_views.data();
    framebuffer_create_info.pNext = nullptr;
    framebuffer_create_info.renderPass = render_pass_;
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
    VkFramebuffer framebuffer;
    const VkResult result = vkCreateFramebuffer(render_context_->GetLogicalDeviceHandle(), &framebuffer_create_info,
                                                nullptr, &framebuffer);
    
    if (result == VK_SUCCESS)
    {
        framebuffers_.push_back(framebuffer);
        return true;
    }

    return false;
}

VkCommandBuffer OpaqueRenderer::RecordCommandBuffers(uint32_t idx)
{
    VkCommandBufferBeginInfo command_buffer_begin_info;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(command_buffers_[idx], &command_buffer_begin_info);

    // Clear color values for color and depth
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkClearValue clear_depth = {1.0f, 0.0f};
    std::array<VkClearValue, 2> clear_values = {clear_color, clear_depth};

    // Render pass
    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.framebuffer = framebuffers_[idx];
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderArea.extent = render_context_->GetSwapchainExtent();
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderPass = render_pass_;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.clearValueCount = 0;
    // render_pass_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffers_[idx], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    // Bind to graphics pipeline
    vkCmdBindPipeline(command_buffers_[idx], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

    // Handle dynamic states of the pipeline
    {
        // Viewport
        VkViewport viewport;
        viewport.height = (float)render_context_->GetSwapchainExtent().height;
        viewport.width = (float)render_context_->GetSwapchainExtent().width;
        viewport.x = 0;
        viewport.y = 0;
        viewport.maxDepth = 1;
        viewport.minDepth = 0;

        vkCmdSetViewport(command_buffers_[idx], 0, 1, &viewport);

        // Scissor
        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = render_context_->GetSwapchainExtent();

        vkCmdSetScissor(command_buffers_[idx], 0, 1, &scissor);
    }

    const VkDeviceSize vertex_offsets = triangle_rendering_data_.vertex_data_offset;
    vkCmdBindVertexBuffers(command_buffers_[idx], 0, 1, &triangle_rendering_data_.buffer, &vertex_offsets);

    const VkDeviceSize indices_offsets = triangle_rendering_data_.indices_offset;
    vkCmdBindIndexBuffer(command_buffers_[idx], triangle_rendering_data_.buffer, indices_offsets, VK_INDEX_TYPE_UINT16);

    // Update mvp matrix
    glm::vec3 camera_pos = {2.0f, 5.0f, -1.0f};
    const glm::mat4 view_matrix = glm::lookAt(camera_pos * -2.f, glm::vec3(0.0f), glm::vec3(0.0f, 1.f, 0.0f));
    const glm::mat4 projection_matrix = glm::perspective(
        65.f, ((float)window_->GetFramebufferSize().width / (float)window_->GetFramebufferSize().height), 0.1f, 200.f);
    const glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    const glm::mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;


    // glm::vec3 camera_pos = {2.0f, 5.0f, -5.0f};
    // glm::mat4 view_matrix = glm::lookAt(camera_pos * -20.f, glm::vec3(0.0f), glm::vec3(0.0f, 0.01f, 0.0f));

    vkCmdPushConstants(command_buffers_[idx], pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp_matrix),
                       &mvp_matrix);

    // Issue Draw command
    vkCmdDrawIndexed(command_buffers_[idx], triangle_rendering_data_.indices_count, 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffers_[idx]);

    const VkResult result = vkEndCommandBuffer(command_buffers_[idx]);

    if (result == VK_SUCCESS)
    {
        return command_buffers_[idx];
    }

    return nullptr;
}

bool OpaqueRenderer::AllocateRenderingResources()
{
    const std::vector<uint16_t> indices = {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        5, 4, 6, 6, 4, 7,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1
    };
    
    const std::vector<VertexData> vertex_data = {
        {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    };

    return render_context_->CreateIndexedRenderingBuffer(indices, vertex_data, command_pool_, triangle_rendering_data_);
}

void OpaqueRenderer::HandleResize(int width, int height)
{
    // Cleanup allocated framebuffer's
    for (const auto framebuffer : framebuffers_)
    {
        vkDestroyFramebuffer(render_context_->GetLogicalDeviceHandle(), framebuffer, nullptr);
    }

    framebuffers_.clear();
    CreateFrameBuffers();
}

bool OpaqueRenderer::CreateRenderPass()
{
    VkAttachmentDescription color_attachment_description;
    color_attachment_description.flags = 0;
    color_attachment_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    // hardcoded for now, we need to ask swapchain instead
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment_description;
    depth_attachment_description.flags = 0;
    depth_attachment_description.format = VK_FORMAT_D32_SFLOAT; // hardcoded for now, we need to ask swapchain instead
    depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    std::array<VkAttachmentDescription, 2> attachment_descriptions{
        color_attachment_description, depth_attachment_description
    };

    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0; // matches to the render pass pAttachments array index
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1; // matches to the render pass pAttachments array index
    depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description{};
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_reference;
    subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.attachmentCount = attachment_descriptions.size();
    render_pass_create_info.dependencyCount = 1; // investigate this
    render_pass_create_info.pAttachments = attachment_descriptions.data();
    render_pass_create_info.pDependencies = &subpass_dependency;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.subpassCount = 1;

    const VkResult result = vkCreateRenderPass(render_context_->GetLogicalDeviceHandle(), &render_pass_create_info,
                                               nullptr, &render_pass_);

    return result == VK_SUCCESS;
}

bool OpaqueRenderer::CreateGraphicsPipeline()
{
    VkPipelineShaderStageCreateInfo vs_shader_stage{};
    VkPipelineShaderStageCreateInfo fs_shader_stage{};

    if (render_context_)
    {
        VkResult result;
        bool vs_shader_create = false;
        bool fs_shader_create = false;

        vs_shader_create = render_context_->CreateShader(R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_vs.spv)",
                                                         VK_SHADER_STAGE_VERTEX_BIT, vs_shader_stage);
        fs_shader_create = render_context_->CreateShader(R"(C:\dev\RabbitHole\src\app\shaders\bytecode\dummy_fs.spv)",
                                                         VK_SHADER_STAGE_FRAGMENT_BIT, fs_shader_stage);

        if (!vs_shader_create || !fs_shader_create)
        {
            return true;
        }

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages = {vs_shader_stage, fs_shader_stage};

        // https://vulkan.lunarg.com/doc/view/1.2.154.1/windows/tutorial/html/08-init_pipeline_layout.html
        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
        descriptor_set_layout_create_info.flags = 0;
        descriptor_set_layout_create_info.bindingCount = 0;
        descriptor_set_layout_create_info.pBindings = nullptr;
        // this is a VkDescriptorSetLayoutBinding for each stage (for uniforms)
        descriptor_set_layout_create_info.pNext = nullptr;
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

        VkDescriptorSetLayout descriptor_set_layout;
        vkCreateDescriptorSetLayout(render_context_->GetLogicalDeviceHandle(), &descriptor_set_layout_create_info,
                                    nullptr, &descriptor_set_layout);

        VkPushConstantRange transform_matrix_push_constant_range;
        transform_matrix_push_constant_range.offset = 0;
        transform_matrix_push_constant_range.size = sizeof(glm::mat4);
        transform_matrix_push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.flags = 0;
        pipeline_layout_create_info.pNext = nullptr;
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pPushConstantRanges = &transform_matrix_push_constant_range;
        pipeline_layout_create_info.pushConstantRangeCount = 1;

        result = vkCreatePipelineLayout(render_context_->GetLogicalDeviceHandle(), &pipeline_layout_create_info,
                                        nullptr, &pipeline_layout_);

        if (result != VK_SUCCESS)
        {
            return false;
        }

        std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info;
        pipeline_dynamic_state_create_info.flags = 0;
        pipeline_dynamic_state_create_info.pNext = nullptr;
        pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipeline_dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
        pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info;
        pipeline_rasterization_state_create_info.flags = 0;
        pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipeline_rasterization_state_create_info.lineWidth = 1.0f;
        pipeline_rasterization_state_create_info.pNext = nullptr;
        pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

        VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info;
        pipeline_viewport_state_create_info.flags = 0;
        pipeline_viewport_state_create_info.pNext = nullptr;
        pipeline_viewport_state_create_info.pScissors = nullptr;
        pipeline_viewport_state_create_info.pViewports = nullptr;
        pipeline_viewport_state_create_info.scissorCount = 1;
        pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipeline_viewport_state_create_info.viewportCount = 1;

        // todo review color blending later
        VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state;
        pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
        pipeline_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
        pipeline_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;

        VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info;
        pipeline_color_blend_state_create_info.flags = 0;
        pipeline_color_blend_state_create_info.attachmentCount = 1;
        pipeline_color_blend_state_create_info.blendConstants[0] = 0;
        pipeline_color_blend_state_create_info.blendConstants[1] = 0;
        pipeline_color_blend_state_create_info.blendConstants[2] = 0;
        pipeline_color_blend_state_create_info.blendConstants[3] = 0;
        pipeline_color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
        pipeline_color_blend_state_create_info.pAttachments = &pipeline_color_blend_attachment_state;
        pipeline_color_blend_state_create_info.pNext = nullptr;
        pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipeline_color_blend_state_create_info.logicOpEnable = VK_FALSE;

        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_create_info;
        pipeline_input_assembly_state_create_info.flags = 0;
        pipeline_input_assembly_state_create_info.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipeline_input_assembly_state_create_info.pNext = nullptr;
        pipeline_input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE; //  what is this??

        // Position input description
        VkVertexInputAttributeDescription vertex_pos_input_attribute_description;
        vertex_pos_input_attribute_description.binding = 0;
        vertex_pos_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
        vertex_pos_input_attribute_description.location = 0;
        vertex_pos_input_attribute_description.offset = offsetof(VertexData, position);

        // Color input description
        VkVertexInputAttributeDescription vertex_color_input_attribute_description;
        vertex_color_input_attribute_description.binding = 0;
        vertex_color_input_attribute_description.format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
        vertex_color_input_attribute_description.location = 1;
        vertex_color_input_attribute_description.offset = offsetof(VertexData, color);

        std::array<VkVertexInputAttributeDescription, 2> vertex_input_attribute_descriptions({
            vertex_pos_input_attribute_description, vertex_color_input_attribute_description
        });

        VkVertexInputBindingDescription vertex_input_binding_description;
        vertex_input_binding_description.binding = 0;
        vertex_input_binding_description.stride = sizeof(VertexData);
        vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info;
        pipeline_vertex_input_state_create_info.flags = 0;
        pipeline_vertex_input_state_create_info.pNext = nullptr;
        pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.
            data();
        pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
        pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_descriptions.
            size();
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

        VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
        pipeline_depth_stencil_state_create_info.flags = 0;
        pipeline_depth_stencil_state_create_info.pNext = nullptr;
        pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
        pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
        pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
        pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;

        // For every subpass we must have pipeline unless they are compatible
        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
        graphics_pipeline_create_info.flags = 0;
        graphics_pipeline_create_info.layout = pipeline_layout_;
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.pStages = shader_stages.data();
        graphics_pipeline_create_info.renderPass = render_pass_;
        graphics_pipeline_create_info.stageCount = shader_stages.size();
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
        graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
        graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
        graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
        graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
        graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_state_create_info;
        graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;

        uint32_t pipeline_count = 1;
        result = vkCreateGraphicsPipelines(render_context_->GetLogicalDeviceHandle(), nullptr, pipeline_count,
                                           &graphics_pipeline_create_info, nullptr, &pipeline_);

        return result == VK_SUCCESS;
    }

    return false;
}

// Note is it a good idea to have this per renderer type? remember VkFramebuffer + VkRenderPass defines a render target
// What are the alternatives? Can we use always the same swapchain framebuffers or should every render type
// have its own framebuffers and at the end combine everything with the swapchain. If so this cant use swapchain images
bool OpaqueRenderer::CreateFrameBuffers()
{
    const int swapchain_image_count = render_context_->GetSwapchainImageCount();
    const VkExtent2D swapchain_extent = render_context_->GetSwapchainExtent();
    const std::vector<SwapchainImage>& swapchain_images = render_context_->GetSwapchainImages();

    for (int i = 0; i < swapchain_image_count; ++i)
    {
        const std::vector<VkImageView> image_views = {
            swapchain_images[i].color_render_target->GetRenderTargetView(), swapchain_images[i].depth_render_target->GetRenderTargetView()
        };

        VkFramebufferCreateInfo framebuffer_create_info;
        framebuffer_create_info.flags = 0;
        framebuffer_create_info.height = swapchain_extent.height;
        framebuffer_create_info.layers = 1;
        framebuffer_create_info.width = swapchain_extent.width;
        framebuffer_create_info.attachmentCount = image_views.size();
        framebuffer_create_info.pAttachments = image_views.data();
        framebuffer_create_info.pNext = nullptr;
        framebuffer_create_info.renderPass = render_pass_;
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

        VkFramebuffer framebuffer;
        const VkResult result = vkCreateFramebuffer(render_context_->GetLogicalDeviceHandle(), &framebuffer_create_info,
                                                    nullptr, &framebuffer);

        if (result == VK_SUCCESS)
        {
            framebuffers_.push_back(framebuffer);
        }
    }

    return framebuffers_.size() == swapchain_image_count;
}
