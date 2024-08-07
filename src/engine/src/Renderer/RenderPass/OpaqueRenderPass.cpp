#include "Renderer/RenderPass/OpaqueRenderPass.hpp"
#include <ext/matrix_clip_space.hpp>
#include <ext/matrix_transform.hpp>

#include "Core/GeometryLoaderSystem.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/RenderGraph/RenderGraph.hpp"
#include "Core/Utils.hpp"

namespace {
    std::string _psoIdentifier = "OpaqueRenderPass_PSO";
    std::string cb_identifier = "OpaqueRenderPass_CB";
}

namespace
{
    static IndexRenderingData triangle_rendering_data_ = {};
}

OpaqueRenderPass::OpaqueRenderPass(RenderGraph* render_graph, OpaquePassDesc* pass_desc, std::string parent_graph_identifier)
    : _passDesc(pass_desc)
    , _pso()
    , _parentGraphIdentifier(parent_graph_identifier)
    , _renderGraph(render_graph)
{
}

bool OpaqueRenderPass::Initialize() {
    if(_renderGraph == nullptr) {
        return false;
    }

    _pso = _renderGraph->GetCachedPSO2(_parentGraphIdentifier + _psoIdentifier);
    if(_pso != nullptr) {
        return true;
    }

    PipelineStateObject pso {};
    _pso =  _renderGraph->RegisterPSO2(_parentGraphIdentifier + _psoIdentifier, pso);
    
    const VkRenderPass render_pass = CreateRenderPass();
    if(render_pass == VK_NULL_HANDLE) {
        return false;
    }

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {};
    const VkPipelineLayout pipeline_layout = CreatePipelineLayout(shader_stages);
    if(pipeline_layout == VK_NULL_HANDLE) {
        return false;
    }

    const VkPipeline pipeline = CreatePipeline(render_pass, pipeline_layout, shader_stages);
    if(pipeline == VK_NULL_HANDLE) {
        return false;
    }

    _pso->render_pass = render_pass;
    _pso->pipeline_layout = pipeline_layout;
    _pso->pipeline = pipeline;
    
    return true;
}

bool OpaqueRenderPass::CreateFramebuffer() {
    if(_renderGraph == nullptr) {
        return false;
    }
    
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(render_context == nullptr) {
        return false;
    }

    if(!_pso) {
        return false;
    }
    
    if(_pso->framebuffer != VK_NULL_HANDLE) {
        return true;
    }
    
    RenderTarget* scene_color = _passDesc->scene_color.get();
    RenderTarget* scene_depth = _passDesc->scene_depth.get();

    if(!scene_color || !scene_depth) {
        return false;
    }
    
    // Make sure that RT's are compatible
    if(scene_color->GetHeight() != scene_depth->GetHeight() || scene_color->GetWidth() != scene_depth->GetWidth()) {
        assert(true && "Incompatible render target sizes");
    }

    const VkExtent2D extent = { scene_color->GetWidth(), scene_color->GetHeight() };
    
    std::vector image_views = {
        scene_color->GetTexture()->GetView(), scene_depth->GetTexture()->GetView()
    };

    VkFramebufferCreateInfo framebuffer_create_info;
    framebuffer_create_info.flags = 0;
    framebuffer_create_info.height = extent.height;
    framebuffer_create_info.layers = 1;
    framebuffer_create_info.width = extent.width;
    framebuffer_create_info.attachmentCount = image_views.size();
    framebuffer_create_info.pAttachments = reinterpret_cast<const VkImageView*>(image_views.data()) ;
    framebuffer_create_info.pNext = nullptr;
    framebuffer_create_info.renderPass = _pso->render_pass;
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    VkFramebuffer framebuffer;
    const VkResult result = VkFunc::vkCreateFramebuffer(render_context->GetLogicalDeviceHandle(), &framebuffer_create_info, nullptr, &framebuffer);

    if(result == VK_SUCCESS) {
        _pso->framebuffer = framebuffer;
        return true;
    }
    
    return false;
}

bool OpaqueRenderPass::CreateCommandBuffer() {
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(render_context == nullptr && !_passDesc && !_passDesc->_commandPool && !_passDesc->_commandPool->GetCommandBuffer()) {
        return false;
    }
    
    // RenderContext* render_context = render_graph_->GetRenderContext();
    // if(render_context == nullptr) {
    //     return false;
    // }
    //
    // pass_resource_ = render_graph_->GetCachedPassResource(parent_graph_identifier_ + pso_identifier);
    //
    // if(pass_resource_ == nullptr) {
    //     render_graph_->RegisterPassResource(parent_graph_identifier_ + pso_identifier, {});
    //     pass_resource_ = render_graph_->GetCachedPassResource(parent_graph_identifier_ + pso_identifier);
    // }
    //
    // Very temporary until we have a better solution to create geometry, this avoid creating it all the time...
    // if(render_context != nullptr) {
    //     static bool geoInitialized = false;
    //     if(!geoInitialized)
    //     {
    //         geoInitialized = true;
    //         std::vector<uint32_t> indices = { 0, 1 ,2 };
    //
    //         std::vector<VertexData> vertex_data = {
    //             {{1.0f,  1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    //             {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    //             {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    //         };
    //
    //         // TODO This is leaking..
    //         render_context->CreateIndexedRenderingBuffer(indices, vertex_data, (VkCommandPool)_passDesc->_commandPool->GetResource(), triangle_rendering_data_);    
    //     }
    // }

    return true;
    //
    // // We already created a command buffer
    // if(pass_resource_->command_buffer != VK_NULL_HANDLE) {
    //     return true;
    // }
    //
    // VkCommandBufferAllocateInfo command_buffer_allocate_info;
    // command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // command_buffer_allocate_info.commandPool = render_graph_->GetCachedCommandPool(parent_graph_identifier_); // TODO pass graph builder instancee and ask for pool
    // command_buffer_allocate_info.pNext = nullptr;
    // command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // command_buffer_allocate_info.commandBufferCount = 1;
    //
    // const VkResult result = VkFunc::vkAllocateCommandBuffers(render_context->GetLogicalDeviceHandle(), &command_buffer_allocate_info, &pass_resource_->command_buffer);
    // if (result == VK_SUCCESS) {
    //     return true;
    // }
    //
    // return false;
}

bool OpaqueRenderPass::RecordCommandBuffer()
{
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(render_context == nullptr && !_passDesc && !_passDesc->_commandPool && !_passDesc->_commandPool->GetCommandBuffer()) {
        return false;
    }

    VkCommandBuffer commandBuffer = (VkCommandBuffer)_passDesc->_commandPool->GetCommandBuffer()->GetResource();
    
    // Clear color values for color and depth
    const float darkness = 0.28f;
    VkClearValue clear_color = {{{0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 1.0}}};
    VkClearValue clear_depth = {1.0f, 0.0f};
    std::array<VkClearValue, 2> clear_values = {clear_color, clear_depth};

    // Render pass
    VkRenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.framebuffer = _pso->framebuffer;
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderArea.extent = render_context->GetSwapchainExtent();
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderPass = _pso->render_pass;
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.clearValueCount = clear_values.size();
    render_pass_begin_info.pClearValues = clear_values.data();

    VkFunc::vkCmdBeginRenderPass(commandBuffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    // Bind to graphics pipeline
    VkFunc::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pso->pipeline);

    // Handle dynamic states of the pipeline
    {
        // Viewport
        VkViewport viewport;
        viewport.height = (float)render_context->GetSwapchainExtent().height;
        viewport.width = (float)render_context->GetSwapchainExtent().width;
        viewport.x = 0;
        viewport.y = 0;
        viewport.maxDepth = 1;
        viewport.minDepth = 0;

        VkFunc::vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // Scissor
        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = render_context->GetSwapchainExtent();

        VkFunc::vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    if(_passDesc->meshNodes && !_passDesc->meshNodes->empty())
    {
        for(const MeshNode& meshNode: *(_passDesc)->meshNodes) {
            GeometryLoaderSystem::ForEachNodeConst(&meshNode, [this, &commandBuffer](const MeshNode* currentNode) {
                int i = 0;
                unsigned int indicesDataOffset = 0;
                unsigned int vertexDataOffset = 0;
            
                for (const PrimitiveData& primitive : currentNode->_primitives) {
                    indicesDataOffset += (primitive._indices.size() * sizeof(unsigned)) * std::clamp(i, 0, 1);

                    auto buffer = (VkBuffer)currentNode->_buffer->GetResource();

                    if(!buffer) {
                        continue;
                    }
                    
                    VkDeviceSize vertexOffset = primitive._indices.size() * sizeof(unsigned) + indicesDataOffset + vertexDataOffset;
                    VkFunc::vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &vertexOffset);
            
                    const VkDeviceSize indicesOffset = indicesDataOffset + vertexDataOffset;
                    VkFunc::vkCmdBindIndexBuffer(commandBuffer, buffer, indicesOffset, VK_INDEX_TYPE_UINT32);
                
                    vertexDataOffset += (primitive._vertexData.size() * sizeof(VertexData)) * std::clamp(i, 0, 1);
                    
                    glm::mat4 model_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
                    model_matrix  = glm::rotate(model_matrix, 90.f, glm::vec3(0.0f, 1.f, 0.0f));
                    const glm::mat4 mvp_matrix = _passDesc->projectionMatrix * _passDesc->viewMatrix * model_matrix;
                    
                    VkFunc::vkCmdPushConstants(commandBuffer, _pso->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mvp_matrix),
                        &mvp_matrix);

                    // Issue Draw command
                    VkFunc::vkCmdDrawIndexed(commandBuffer, primitive._indices.size(), 1, 0, 0, 0);
                    
                    i++;
                }
            });    
        }    
    }
    
    VkFunc::vkCmdEndRenderPass(commandBuffer);

    return true;
}

std::vector<VkCommandBuffer> OpaqueRenderPass::GetCommandBuffers() {
    return {};
}

VkRenderPass OpaqueRenderPass::CreateRenderPass() {
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(!render_context) {
        return VK_NULL_HANDLE;
    }
    
    VkAttachmentDescription color_attachment_description;
    color_attachment_description.flags = 0;
    color_attachment_description.format = VK_FORMAT_B8G8R8A8_SRGB;
    // hardcoded for now, we need to ask swapchain instead
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::cout << "ColorAttachmentDescription" << std::endl;
    std::cout << hash_value(color_attachment_description) << std::endl;

    VkAttachmentDescription depth_attachment_description;
    depth_attachment_description.flags = 0;
    depth_attachment_description.format = VK_FORMAT_D32_SFLOAT; // hardcoded for now, we need to ask swapchain instead
    depth_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depth_attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.attachmentCount = attachment_descriptions.size();
    render_pass_create_info.dependencyCount = 1; // investigate this
    render_pass_create_info.pAttachments = attachment_descriptions.data();
    render_pass_create_info.pDependencies = &subpass_dependency;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.subpassCount = 1;

    VkRenderPass render_pass;
    VkFunc::vkCreateRenderPass(render_context->GetLogicalDeviceHandle(), &render_pass_create_info, nullptr, &render_pass);

    return render_pass;
}

VkPipelineLayout OpaqueRenderPass::CreatePipelineLayout(std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages) {
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(!render_context) {
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo vs_shader_stage{};
    VkPipelineShaderStageCreateInfo fs_shader_stage{};
        
    const bool vs_shader_create = render_context->CreateShader(_passDesc->vs_shader_._source, ShaderStage::STAGE_VERTEX, vs_shader_stage);
    const bool fs_shader_create = render_context->CreateShader(_passDesc->ps_shader._source, ShaderStage::STAGE_FRAGMENT, fs_shader_stage);

    if (!vs_shader_create || !fs_shader_create) {
        std::cerr << "[Error]: Unable to create shaders." << std::endl;
        return VK_NULL_HANDLE;
    }

    shader_stages = {vs_shader_stage, fs_shader_stage};

    // https://vulkan.lunarg.com/doc/view/1.2.154.1/windows/tutorial/html/08-init_pipeline_layout.html
    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info;
    descriptor_set_layout_create_info.flags = 0;
    descriptor_set_layout_create_info.bindingCount = 0;
    descriptor_set_layout_create_info.pBindings = nullptr;
    // this is a VkDescriptorSetLayoutBinding for each stage (for uniforms)
    descriptor_set_layout_create_info.pNext = nullptr;
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    VkDescriptorSetLayout descriptor_set_layout;
    VkFunc::vkCreateDescriptorSetLayout(render_context->GetLogicalDeviceHandle(), &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);

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

    VkPipelineLayout pipeline_layout;
    VkFunc::vkCreatePipelineLayout(render_context->GetLogicalDeviceHandle(), &pipeline_layout_create_info, nullptr, &pipeline_layout);

    return pipeline_layout;
}

VkPipeline OpaqueRenderPass::CreatePipeline(VkRenderPass render_pass, VkPipelineLayout pipeline_layout, const std::array<VkPipelineShaderStageCreateInfo, 2>& shader_stages) {
    RenderContext* render_context = _renderGraph->GetRenderContext();
    if(!render_context) {
        return VK_NULL_HANDLE;
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
    pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
    pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipeline_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipeline_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

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

    std::array<VkVertexInputAttributeDescription, 2> vertex_input_attribute_descriptions({ vertex_pos_input_attribute_description, vertex_color_input_attribute_description });

    VkVertexInputBindingDescription vertex_input_binding_description;
    vertex_input_binding_description.binding = 0;
    vertex_input_binding_description.stride = sizeof(VertexData);
    vertex_input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info;
    pipeline_vertex_input_state_create_info.flags = 0;
    pipeline_vertex_input_state_create_info.pNext = nullptr;
    pipeline_vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_input_attribute_descriptions.data();
    pipeline_vertex_input_state_create_info.pVertexBindingDescriptions = &vertex_input_binding_description;
    pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_input_attribute_descriptions.size();
    pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount = 1;

    VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info{};
    pipeline_depth_stencil_state_create_info.flags = 0;
    pipeline_depth_stencil_state_create_info.pNext = nullptr;
    pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipeline_depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipeline_depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
    pipeline_depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
    pipeline_depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
    pipeline_depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;

    // For every subpass we must have pipeline unless they are compatible
    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.flags = 0;
    graphics_pipeline_create_info.layout = pipeline_layout;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.pStages = shader_stages.data();
    graphics_pipeline_create_info.renderPass = render_pass;
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
    VkPipeline pipeline;
    VkFunc::vkCreateGraphicsPipelines(render_context->GetLogicalDeviceHandle(), nullptr, pipeline_count, &graphics_pipeline_create_info, nullptr, &pipeline);

    return pipeline;
}
