#include "Renderer/RenderPass/FullScreenQuadRenderPass.hpp"

#include "Renderer/CommandBuffer.hpp"
#include "Renderer/RenderTarget.hpp"

namespace {
    const char* psoIdentifier = "FullScreenQuadRenderPass_PSO";    
}

FullScreenQuadRenderPass::FullScreenQuadRenderPass(RenderGraph* renderGraph, FullScreenQuadPassDesc* passDesc, std::string parentGraphIdentifier)
    : _renderGraph(renderGraph)
    , _passDesc(passDesc)
    , _pso(nullptr)
    , _parentGraphIdentifier(std::move(parentGraphIdentifier))
{
}

bool FullScreenQuadRenderPass::Initialize() {
    if(_renderGraph == nullptr) {
        return false;
    }

    _pso = _renderGraph->GetCachedPSO2(_parentGraphIdentifier + psoIdentifier);
    if(_pso != nullptr) {
        return true;
    }

    PipelineStateObject pso {};
    _pso =  _renderGraph->RegisterPSO2(_parentGraphIdentifier + psoIdentifier, pso);

    const VkRenderPass renderPass = CreateRenderPass();
    if(renderPass == VK_NULL_HANDLE) {
        return false;
    }
    
    const auto [pipeline, pipelineLayout] = CreateGraphicsPipeline(renderPass);
    if(pipeline == VK_NULL_HANDLE) {
        return false;
    }
    
    _pso->pipeline = pipeline;
    _pso->pipeline_layout = pipelineLayout;
    _pso->render_pass = renderPass;
    
    return true;
}

bool FullScreenQuadRenderPass::CreateFramebuffer() {
    if(_renderGraph == nullptr) {
        return false;
    }

    const RenderContext* render_context = _renderGraph->GetRenderContext();
    if(render_context == nullptr) {
        return false;
    }

    if(!_pso) {
        return false;
    }
    
    if(_pso->framebuffer != VK_NULL_HANDLE) {
        return true;
    }

    RenderTarget* sceneColor = _passDesc->sceneColor.get();
    RenderTarget* sceneDepth = _passDesc->sceneDepth.get();

    if(!sceneColor || !sceneDepth) {
        return false;
    }

    // Make sure that RT's are compatible
    if(sceneColor->GetHeight() != sceneDepth->GetHeight() || sceneColor->GetWidth() != sceneDepth->GetWidth()) {
        assert(true && "Incompatible render target sizes");
    }
    
    const VkExtent2D extent = { sceneColor->GetWidth(), sceneColor->GetHeight() };
    std::vector imageViews = { sceneColor->GetView(), sceneDepth->GetView()  };

    VkFramebufferCreateInfo frameBufferCreateInfo {};
    frameBufferCreateInfo.flags = 0;
    frameBufferCreateInfo.height = extent.height;
    frameBufferCreateInfo.layers = 1;
    frameBufferCreateInfo.width = extent.width;
    frameBufferCreateInfo.attachmentCount = imageViews.size();
    frameBufferCreateInfo.pAttachments = reinterpret_cast<VkImageView*>(imageViews.data());
    frameBufferCreateInfo.pNext = nullptr;
    frameBufferCreateInfo.renderPass = _pso->render_pass;
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    VkFramebuffer framebuffer;
    const VkResult result = VkFunc::vkCreateFramebuffer(render_context->GetLogicalDeviceHandle(), &frameBufferCreateInfo, nullptr, &framebuffer);
    if(result != VK_SUCCESS) {
        return false;
    }

    _pso->framebuffer = framebuffer;
    
    return true;
}

bool FullScreenQuadRenderPass::CreateCommandBuffer() {
    return true;
}

bool FullScreenQuadRenderPass::RecordCommandBuffer()
{
    if(_renderGraph == nullptr) {
        return false;
    }
    
    RenderContext* renderContext = _renderGraph->GetRenderContext();
    if(renderContext == nullptr || _passDesc == nullptr) {
        return false;
    }

    if(_passDesc->_commandPool == nullptr) {
        return false;
    }

    VkCommandBuffer commandBuffer = (VkCommandBuffer)_passDesc->_commandPool->GetCommandBuffer()->GetResource();

    // Clear color values for color and depth
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    VkClearValue clearDepth = {1.0f, 0.0f};
    std::array<VkClearValue, 2> clear_values = {clearColor, clearDepth};

    // Render pass
    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.framebuffer = _pso->framebuffer;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderArea.extent = renderContext->GetSwapchainExtent();
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderPass = _pso->render_pass;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clear_values.data();
    
    VkFunc::vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Bind to graphics pipeline
    VkFunc::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pso->pipeline);

    // Handle dynamic states of the pipeline
    {
        // Viewport
        VkViewport viewport;
        viewport.height = (float)renderContext->GetSwapchainExtent().height;
        viewport.width = (float)renderContext->GetSwapchainExtent().width;
        viewport.x = 0;
        viewport.y = 0;
        viewport.maxDepth = 1;
        viewport.minDepth = 0;

        VkFunc::vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // Scissor
        VkRect2D scissor;
        scissor.offset = {0, 0};
        scissor.extent = renderContext->GetSwapchainExtent();

        VkFunc::vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    VkFunc::vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pso->pipeline_layout, 0, 1, &_pso->descriptorSet, 0, nullptr);

    // Issue Draw command
    VkFunc::vkCmdDraw(commandBuffer, 6, 1, 0, 0);

    VkFunc::vkCmdEndRenderPass(commandBuffer);

    // Assume that you have acquired the next available swapchain image and stored it in the VkImage variable called "swapchainImage"

    // // Define the image memory barrier to transition the image from the color attachment layout to the present layout
    // VkImageMemoryBarrier barrier {};
    // barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    // barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    // barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    // barrier.image = _passDesc->sceneColor()->GetResource();
    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // barrier.subresourceRange.baseMipLevel = 0;
    // barrier.subresourceRange.levelCount = 1;
    // barrier.subresourceRange.baseArrayLayer = 0;
    // barrier.subresourceRange.layerCount = 1;
    //
    // // Submit a pipeline barrier to transition the image layout
    // VkFunc::vkCmdPipelineBarrier(
    //   commandBuffer, 
    //   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
    //   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
    //   0, 
    //   0, nullptr, 
    //   0, nullptr, 
    //   1, &barrier
    // );


    return true;
}

std::vector<VkCommandBuffer> FullScreenQuadRenderPass::GetCommandBuffers()
{
    return {};
}

VkRenderPass FullScreenQuadRenderPass::CreateRenderPass() const {
    const RenderContext* renderContext = _renderGraph->GetRenderContext();
    if(!renderContext) {
        return VK_NULL_HANDLE;
    }

    // TODO Try to get the most information from the texture, things like samples and format
    VkAttachmentDescription colorAttachmentDescription {};
    colorAttachmentDescription.flags = 0;
    colorAttachmentDescription.format = VK_FORMAT_B8G8R8A8_SRGB;
    colorAttachmentDescription.samples =  VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    // colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachmentDescription {};
    depthAttachmentDescription.flags = 0;
    depthAttachmentDescription.format = VK_FORMAT_D32_SFLOAT; // hardcoded for now, we need to ask swapchain instead
    depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    // depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentDescription, 2> attachmentDescriptions {
        colorAttachmentDescription, depthAttachmentDescription
    };
    
    VkAttachmentReference colorAttachmentReference {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentReference {};
    depthAttachmentReference.attachment = 1; // matches to the render pass pAttachments array index
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPassDescription {};
    subPassDescription.colorAttachmentCount = 1;
    subPassDescription.pColorAttachments = &colorAttachmentReference;
    subPassDescription.pDepthStencilAttachment = &depthAttachmentReference;
    subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    VkSubpassDependency subPassDependency {};
    subPassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subPassDependency.dstSubpass = 0;
    subPassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subPassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subPassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subPassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo {};
    renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.pSubpasses = &subPassDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subPassDependency;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    VkRenderPass renderPass;
    VkFunc::vkCreateRenderPass(renderContext->GetLogicalDeviceHandle(), &renderPassCreateInfo, nullptr, &renderPass);

    return renderPass;
}

VkPipelineLayout FullScreenQuadRenderPass::CreatePipelineLayout(VkRenderPass renderPass, std::array<VkPipelineShaderStageCreateInfo, 2>& shaderStages) const {
    const RenderContext* renderContext = _renderGraph->GetRenderContext();
    if(!renderContext) {
        return VK_NULL_HANDLE;
    }

    VkPipelineShaderStageCreateInfo vsShaderStage{};
    VkPipelineShaderStageCreateInfo fsShaderStage{};

    const bool bValidVSShader = renderContext->CreateShader(_passDesc->vs_shader_._source, VK_SHADER_STAGE_VERTEX_BIT, vsShaderStage);
    const bool bValidFSShader = renderContext->CreateShader(_passDesc->ps_shader._source, VK_SHADER_STAGE_FRAGMENT_BIT, fsShaderStage);

    if (!bValidVSShader || !bValidFSShader) {
        return VK_NULL_HANDLE;
    }

    shaderStages = { vsShaderStage, fsShaderStage };
    
    VkSamplerCreateInfo samplerCreateInfo {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 1.0f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerCreateInfo.mipLodBias = 0.0f;

    VkSampler sampler;
    VkResult result = VkFunc::vkCreateSampler(renderContext->GetLogicalDeviceHandle(), &samplerCreateInfo, nullptr, &sampler);
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    // For the screen quad texture binding
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding {};
    descriptorSetLayoutBinding.binding = 0;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    descriptorSetLayoutBinding.pImmutableSamplers = &sampler;
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {};
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    VkDescriptorSetLayout descriptorSetLayout;
    VkFunc::vkCreateDescriptorSetLayout(renderContext->GetLogicalDeviceHandle(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
    
    result = VkFunc::vkCreateDescriptorSetLayout(renderContext->GetLogicalDeviceHandle(), &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    // Create a pool for descriptor sets
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 1;
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &poolSize;
    VkDescriptorPool descriptorPool;
    result = VkFunc::vkCreateDescriptorPool(renderContext->GetLogicalDeviceHandle(), &descriptorPoolCreateInfo, nullptr, &descriptorPool);
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    // Allocate a descriptor set from the pool
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    VkDescriptorSet descriptorSet;
    result = VkFunc::vkAllocateDescriptorSets(renderContext->GetLogicalDeviceHandle(), &allocInfo, &descriptorSet);
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    _pso->descriptorSet = descriptorSet;
    
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = static_cast<VkImageView>(_passDesc->texture->GetView());
    imageInfo.sampler = sampler;
    
    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;
    VkFunc::vkUpdateDescriptorSets(renderContext->GetLogicalDeviceHandle(), 1, &descriptorWrite, 0, nullptr);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    
    VkPipelineLayout pipelineLayout;
    result = VkFunc::vkCreatePipelineLayout(renderContext->GetLogicalDeviceHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if(result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    
    return pipelineLayout;
}

std::pair<VkPipeline, VkPipelineLayout> FullScreenQuadRenderPass::CreateGraphicsPipeline(VkRenderPass renderPass) const {
    const RenderContext* renderContext = _renderGraph->GetRenderContext();
    if(!renderContext) {
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }
    
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {};
    const VkPipelineLayout pipelineLayout = CreatePipelineLayout(renderPass, shaderStages);
    if(pipelineLayout == VK_NULL_HANDLE) {
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    const std::array dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.flags = 0;
    dynamicStateCreateInfo.pNext = nullptr;
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo {};
    rasterizationCreateInfo.flags = 0;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationCreateInfo.lineWidth = 1.0f;
    rasterizationCreateInfo.pNext = nullptr;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.depthBiasClamp = 0.0f;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizationCreateInfo.depthBiasSlopeFactor = 0.0f;
    
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    viewportStateCreateInfo.flags = 0;
    viewportStateCreateInfo.pNext = nullptr;
    viewportStateCreateInfo.pScissors = nullptr;
    viewportStateCreateInfo.pViewports = nullptr;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo {};
    colorBlendStateCreateInfo.flags = 0;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.blendConstants[0] = 0;
    colorBlendStateCreateInfo.blendConstants[1] = 0;
    colorBlendStateCreateInfo.blendConstants[2] = 0;
    colorBlendStateCreateInfo.blendConstants[3] = 0;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.pNext = nullptr;
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo {};
    inputAssemblyStateCreateInfo.flags = 0;
    inputAssemblyStateCreateInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyStateCreateInfo.pNext = nullptr;
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; //  what is this??

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo {};
    depthStencilStateCreateInfo.flags = 0;
    depthStencilStateCreateInfo.pNext = nullptr;
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_FALSE;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
     
    // For every subpass we must have pipeline unless they are compatible
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.pStages = shaderStages.data();
    graphicsPipelineCreateInfo.stageCount = shaderStages.size();
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;

    uint32_t pipeline_count = 1;
    VkPipeline pipeline;
    VkResult result = VkFunc::vkCreateGraphicsPipelines(renderContext->GetLogicalDeviceHandle(), nullptr, pipeline_count, &graphicsPipelineCreateInfo, nullptr, &pipeline);
    if(result != VK_SUCCESS) {
        return {VK_NULL_HANDLE, VK_NULL_HANDLE};
    }

    return {pipeline, pipelineLayout};
}
