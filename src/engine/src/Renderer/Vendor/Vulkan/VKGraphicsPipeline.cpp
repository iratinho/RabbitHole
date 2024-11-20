#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VKDevice.hpp"
#include "Renderer/Vendor/Vulkan/VKTextureView.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/Vulkan/VulkanTranslator.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/RenderPass/RenderPassInterface.hpp"

// TODO REMOVE THIS ASAP, JUST HERE FOR UTILITIES

#include "glm/ext.hpp"

void VKGraphicsPipeline::Compile() {
    if(_bWasCompiled)
        return;
    
    CompileShaders();
    
    VKShader* vShader = (VKShader*)(_vertexShader.get());
    VKShader* fShader = (VKShader*)(_fragmentShader.get());
            
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;

    VkPushConstantRange constantRange = BuildPushConstants();
    if(constantRange.size > 0) {
        pipelineLayoutCreateInfo.pPushConstantRanges = &constantRange;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    }
    
    std::vector<VkDescriptorSetLayout> descriptorLayouts = BuildDescriptorSetLayouts();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts.data();
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<unsigned int>(descriptorLayouts.size());
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    
    VkResult result = VkFunc::vkCreatePipelineLayout(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout);
    
    if (result != VK_SUCCESS) {
        assert(0);
        return;
    }
    
    std::array<VkDynamicState, 2> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
    pipelineDynamicStateCreateInfo.flags = 0;
    pipelineDynamicStateCreateInfo.pNext = nullptr;
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.dynamicStateCount = dynamic_states.size();
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamic_states.data();
    
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
    pipelineViewportStateCreateInfo.flags = 0;
    pipelineViewportStateCreateInfo.pNext = nullptr;
    pipelineViewportStateCreateInfo.pScissors = nullptr;
    pipelineViewportStateCreateInfo.pViewports = nullptr;
    pipelineViewportStateCreateInfo.scissorCount = 1;
    pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCreateInfo.viewportCount = 1;
    
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.flags = 0;
    pipelineRasterizationStateCreateInfo.cullMode = TranslateCullMode(_params._rasterization._triangleCullMode);
    pipelineRasterizationStateCreateInfo.frontFace = TranslateWindingOrder(_params._rasterization._triangleWindingOrder);
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.pNext = nullptr;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.depthBiasClamp = _params._rasterization._depthBiasClamp;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = _params._rasterization._depthBias;
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = _params._rasterization._depthBiasSlope;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
    pipelineInputAssemblyStateCreateInfo.flags = 0;
    pipelineInputAssemblyStateCreateInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
    pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; //  what is this??
        
    std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentStates = CreateColorBlendAttachemnt();
    
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
    pipelineColorBlendStateCreateInfo.flags = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[0] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[1] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[2] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[3] = 0;
    pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    pipelineColorBlendStateCreateInfo.pAttachments = colorAttachmentStates.data();
    pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<unsigned int>(colorAttachmentStates.size());
    pipelineColorBlendStateCreateInfo.pNext = nullptr;
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    
    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo {};
    pipelineDepthStencilStateCreateInfo.flags = 0;
    pipelineDepthStencilStateCreateInfo.pNext = nullptr;
    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineDepthStencilStateCreateInfo.depthCompareOp = TranslateCompareOP(_params._rasterization._depthCompareOP);
    pipelineDepthStencilStateCreateInfo.depthTestEnable = HasDepthAttachments();
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = HasDepthAttachments();
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;


    VkRenderPass renderPass = CreateRenderPass();

    if (renderPass == VK_NULL_HANDLE) {
        assert(0);
        return;
    }

    VkPipelineMultisampleStateCreateInfo multiSampleCreateInfo {};
    multiSampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multiSampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multiSampleCreateInfo.minSampleShading = 1.0f;
    multiSampleCreateInfo.pSampleMask = nullptr;
    multiSampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multiSampleCreateInfo.alphaToOneEnable = VK_FALSE;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos = {
        vShader->GetShaderStageInfo(),
        fShader->GetShaderStageInfo()
    };
    
    if(shaderStageInfos.size() != 2) {
        assert(0);
        return;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    auto[inputBindingDescriptors, inputAttributesDescriptors] = BuildVertexStateData();
    
    // There is no vertex data
    if(inputBindingDescriptors.empty()) {
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; // Correct sType
        vertexInputInfo.vertexBindingDescriptionCount = 0;  // No vertex bindings
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // No bindings
        vertexInputInfo.vertexAttributeDescriptionCount = 0; // No vertex attributes
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // No attributes
    } else {
        vertexInputInfo.flags = 0;
        vertexInputInfo.pNext = nullptr;
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pVertexAttributeDescriptions = inputAttributesDescriptors.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(inputAttributesDescriptors.size());
        vertexInputInfo.pVertexBindingDescriptions = inputBindingDescriptors.data();
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<unsigned int>(inputBindingDescriptors.size());
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.layout = _pipelineLayout;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.pStages = shaderStageInfos.data();
    graphicsPipelineCreateInfo.stageCount = static_cast<unsigned int>(shaderStageInfos.size());
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multiSampleCreateInfo;
    
    uint32_t pipelineCount = 1;
    result = VkFunc::vkCreateGraphicsPipelines(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), nullptr, pipelineCount, &graphicsPipelineCreateInfo, nullptr, &_pipeline);

    if (result != VK_SUCCESS) {
        assert(0);
        return;
    }
    
    _bWasCompiled = true;
}

std::vector<VkPipelineColorBlendAttachmentState> VKGraphicsPipeline::CreateColorBlendAttachemnt() {
    std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentStates;
    
    ColorAttachmentBinding& colorAttachmentBinding = _params._renderAttachments._colorAttachmentBinding;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    colorBlendAttachmentState.blendEnable = true; // Always assume we have blending, this will crash if not. Fix me later
    colorBlendAttachmentState.colorBlendOp = TranslateBlendOperation(colorAttachmentBinding._blending._colorBlending);
    colorBlendAttachmentState.alphaBlendOp = TranslateBlendOperation(colorAttachmentBinding._blending._alphaBlending);
    colorBlendAttachmentState.srcColorBlendFactor = TranslateBlendFactor(colorAttachmentBinding._blending._colorBlendingFactor._srcBlendFactor);
    colorBlendAttachmentState.dstColorBlendFactor = TranslateBlendFactor(colorAttachmentBinding._blending._colorBlendingFactor._dstBlendFactor);
    colorBlendAttachmentState.srcAlphaBlendFactor = TranslateBlendFactor(colorAttachmentBinding._blending._alphaBlendingFactor._srcBlendFactor);
    colorBlendAttachmentState.dstAlphaBlendFactor = TranslateBlendFactor(colorAttachmentBinding._blending._alphaBlendingFactor._dstBlendFactor);
    
    colorAttachmentStates.push_back(colorBlendAttachmentState);

    return colorAttachmentStates;
}

std::vector<VkAttachmentDescription> VKGraphicsPipeline::CreateAttachmentDescriptions() {
    std::vector<VkAttachmentDescription> attachmentDescriptors;

    bool bHasDepthAttachments = HasDepthAttachments();

    VkAttachmentDescription colorAttachmentDesc {};
    VkAttachmentDescription depthAttachmentDesc {};
    
    // Color attachment
    {
        ColorAttachmentBinding& colorAttachmentBinding = _params._renderAttachments._colorAttachmentBinding;

        colorAttachmentDesc.initialLayout = TranslateImageLayout(ImageLayout::LAYOUT_COLOR_ATTACHMENT); // For now always assume the layout
        colorAttachmentDesc.finalLayout = TranslateImageLayout(ImageLayout::LAYOUT_COLOR_ATTACHMENT); // For now always assume the layout
        
        colorAttachmentDesc.loadOp = TranslateLoadOP(colorAttachmentBinding._loadAction);
        colorAttachmentDesc.storeOp = TranslateStoreOP(StoreOp::OP_STORE); // For now assume that every render pass wants to store its results
//        colorAttachmentDesc.stencilLoadOp = TranslateLoadOP(loadStoreOps.stencilLoad);
//        colorAttachmentDesc.stencilStoreOp = TranslateStoreOP(loadStoreOps.stencilStore);
        
        colorAttachmentDesc.format = TranslateFormat(colorAttachmentBinding._texture->GetPixelFormat());
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.flags = 0;

        attachmentDescriptors.push_back(colorAttachmentDesc);
    }
    
    // Depth attachment
    if(bHasDepthAttachments) {
        DepthStencilAttachmentBinding& depthStencilAttachmentBinding = _params._renderAttachments._depthStencilAttachmentBinding.value();
        
        depthAttachmentDesc.initialLayout = TranslateImageLayout(ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);
        depthAttachmentDesc.finalLayout = TranslateImageLayout(ImageLayout::LAYOUT_DEPTH_STENCIL_ATTACHMENT);

        depthAttachmentDesc.loadOp = TranslateLoadOP(depthStencilAttachmentBinding._depthLoadAction);
        depthAttachmentDesc.storeOp = TranslateStoreOP(StoreOp::OP_STORE); // For now assume that every render pass wants to store its results
        depthAttachmentDesc.stencilLoadOp = TranslateLoadOP(depthStencilAttachmentBinding._stencilLoadAction);
        depthAttachmentDesc.stencilStoreOp = TranslateStoreOP(depthStencilAttachmentBinding._stencilStoreAction);
        depthAttachmentDesc.format = TranslateFormat(depthStencilAttachmentBinding._texture->GetPixelFormat());
        depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDesc.flags = 0;

        attachmentDescriptors.push_back(depthAttachmentDesc);
    }

    return attachmentDescriptors;
}

std::vector<VkAttachmentReference> VKGraphicsPipeline::CreateColorAttachmentRef() {
    std::vector<VkAttachmentReference> colorAttachmentRefs;
    
    // For now we only have one color attachment but this will change
    VkAttachmentReference attachmentReference{};
    attachmentReference.attachment = static_cast<unsigned int>(0);
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorAttachmentRefs.push_back(attachmentReference);
    
    return colorAttachmentRefs;
}

VkAttachmentReference VKGraphicsPipeline::CreateDepthAttachmentRef() {
    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 1; //_colorAttachments.size();
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    return depthAttachmentRef;
}

VkRenderPass VKGraphicsPipeline::CreateRenderPass() {
    std::vector<VkAttachmentDescription> attachmentDescriptors = CreateAttachmentDescriptions();
    std::vector<VkAttachmentReference> colorAttachmentRefs = CreateColorAttachmentRef();
    VkAttachmentReference depthAttachmentRef = CreateDepthAttachmentRef();

    /*
    * NOTE: At the moment we are not able to infer subpass information, for now
    * we assume the most basic setup (1 subpass), but this information should be
    * grouped with attachments for example, a set of attachments belong to a
    * subpass. src and dst masks should also be specified in this group...
    */

    VkSubpassDescription subpassDescription{};
    subpassDescription.pColorAttachments = colorAttachmentRefs.data();
    subpassDescription.colorAttachmentCount = static_cast<unsigned int>(colorAttachmentRefs.size());
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.flags = 0;

    if(HasDepthAttachments()) {
        subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
    }


    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Depth
    if (HasDepthAttachments()) {
        subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pAttachments = attachmentDescriptors.data();
    renderPassCreateInfo.attachmentCount = static_cast<unsigned int>(attachmentDescriptors.size());
    renderPassCreateInfo.pDependencies = &subpassDependency;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VkResult result = VkFunc::vkCreateRenderPass(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &renderPassCreateInfo, nullptr, &_renderPass);

    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return _renderPass;
}

/*
 * A bit obscure but lets see.
 *
 * We have multiple in-flight frames, we create a unique pipeline and render pass
 *  that is shared betwen the frames. The framebuffers cant be shared, but currently
 *  we store the framebuffers with the pipeline since there is a strong relation with
 *  its render pass. A framebuffer can use compatible render pass objects.
 *
 *  So we map framebuffers per image views, meaning that we are trying to create a framebuffer
 *  per unique imageviews sets, everytime a shared render pass that uses diferent attachments
 *  we must create a new framebuffer
 *
 *  Consider moving this to the command encoder, since it will be more at the render pass level
 */
VkFramebuffer VKGraphicsPipeline::CreateFrameBuffer(std::vector<Texture2D*> textures) {
    uint32_t width = 0;
    uint32_t height = 0;
    
    std::vector<VkImageView> imageViews;
    for(auto texture : textures) {
        VKTextureView* textureView = (VKTextureView*)texture->MakeTextureView();
        if(textureView) {
            VkImageView imageView = (VkImageView)textureView->GetImageView();
            
            if(imageView) {
                bool bAlreadyHasView = std::find(_views.begin(), _views.end(), imageView) != _views.end();
                
                if(!bAlreadyHasView) {
                    imageViews.push_back(imageView);
                }
            }
        }
        
        width = std::max(width, texture->GetWidth());
        height = std::max(height, texture->GetHeight());
    }
    
    bool bIsDirty = false;
    
    const uint32_t hash = hash_value(imageViews);
    
    if(_frameBuffers.find(hash) != _frameBuffers.end()) {
        return _frameBuffers[hash];
    }
    
    VkFramebufferCreateInfo frameBufferInfo {};
    frameBufferInfo.width = width;
    frameBufferInfo.height = height;
    frameBufferInfo.attachmentCount = imageViews.size(); // For now until we support tile rendering
    frameBufferInfo.pAttachments = imageViews.data();
    frameBufferInfo.pNext = VK_NULL_HANDLE;
    frameBufferInfo.renderPass = _renderPass;
    frameBufferInfo.flags = 0;
    frameBufferInfo.layers = 1;
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    
//    _views.insert(_views.end(), imageViews.begin(), imageViews.end());
    
    VkFramebuffer frameBuffer = VK_NULL_HANDLE;
    if(VkFunc::vkCreateFramebuffer(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &frameBufferInfo, VK_NULL_HANDLE, &frameBuffer) != VK_SUCCESS) {
        std::cerr << "Unable to create vulkan framebuffer" << std::endl;
        return nullptr;
    }
    
    _frameBuffers[hash] = frameBuffer;

    // Most likelly the first time we create a framebuffer with this pipeline
//    if(_views.empty()) {
//        _views = imageViews;
//        if(VkFunc::vkCreateFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &frameBufferInfo, VK_NULL_HANDLE, &_frameBuffer) != VK_SUCCESS) {
//            std::cerr << "Unable to create vulkan framebuffer" << std::endl;
//            return nullptr;
//        }
//    }
    
//    // We already had cached image views, lets check if the new ones match with the ones we have
//    // in case it wont match it means that we need to create a new framebuffer
//    if(_views != imageViews) {
//        VkFunc::vkDestroyFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), _frameBuffer, VK_NULL_HANDLE);
//        VkFunc::vkCreateFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &frameBufferInfo, VK_NULL_HANDLE, &_frameBuffer);
//    }
    
    return frameBuffer;
}

void VKGraphicsPipeline::DestroyFrameBuffer() {
    for(auto [hash, frameBuffer] : _frameBuffers) {
        VkFunc::vkDestroyFramebuffer(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), frameBuffer, VK_NULL_HANDLE);

    }
//    VkFunc::vkDestroyFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), _frameBuffer, VK_NULL_HANDLE);
    _frameBuffers.clear();
}

VKGraphicsPipeline::VertexStateData VKGraphicsPipeline::BuildVertexStateData() {
    const ShaderInputBindings& inputBindings = _params._renderPass->CollectShaderInputBindings();
    
    std::vector<VkVertexInputBindingDescription> inputBindingDescriptors;
    std::vector<VkVertexInputAttributeDescription> inputAttributesDescriptors;
    
    for(auto& [key, value] : inputBindings) {
        VkVertexInputBindingDescription bindingDescriptor {};
        bindingDescriptor.binding = key._binding;
        bindingDescriptor.stride = key._stride;
        bindingDescriptor.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescriptors.push_back(bindingDescriptor);
        
        int i = 0;
        for(auto& location : value) {
            VkVertexInputAttributeDescription attributeDescriptor {};
            attributeDescriptor.binding = key._binding;
            attributeDescriptor.location = i;
            attributeDescriptor.offset = location._offset;
            attributeDescriptor.format = TranslateFormat(location._format);
            inputAttributesDescriptors.push_back(attributeDescriptor);
            
            i++;
        }
    }
    
    return std::make_pair(inputBindingDescriptors, inputAttributesDescriptors);
}

VkPushConstantRange VKGraphicsPipeline::BuildPushConstants() {
    VkPushConstantRange constantRange;
    constantRange.size = 0;
    constantRange.offset = 0;
    constantRange.stageFlags = 0;
    
    const std::vector<ShaderDataStream>& dataStreams = _params._renderPass->CollectShaderDataStreams();
    for(const ShaderDataStream& dataStream : dataStreams) {
        if(dataStream._usage != ShaderDataStreamUsage::PUSH_CONSTANT) {
            continue;
        }
        
        for(const ShaderDataBlock& dataBlock : dataStream._dataBlocks) {
            constantRange.size += dataBlock._size;
            constantRange.stageFlags |= TranslateShaderStage(dataBlock._stage);
        }
    }

    return constantRange;
}

std::vector<VkDescriptorSetLayout> VKGraphicsPipeline::BuildDescriptorSetLayouts() {
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

    const std::vector<ShaderDataStream>& dataStreams = _params._renderPass->CollectShaderDataStreams();
    for(const ShaderDataStream& dataStream : dataStreams) {
        if(dataStream._usage != ShaderDataStreamUsage::DATA) {
            continue;
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

        unsigned int binding = 0;
        for(const ShaderDataBlock& dataBlock : dataStream._dataBlocks) {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding {};
            descriptorSetLayoutBinding.binding = binding;
            descriptorSetLayoutBinding.descriptorCount = 1;
            descriptorSetLayoutBinding.stageFlags = TranslateShaderStage(dataBlock._stage);

            if(dataBlock._usage == ShaderDataBlockUsage::UNIFORM_BUFFER) {
                descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }
            
            if(dataBlock._usage == ShaderDataBlockUsage::TEXTURE) {
                descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
            }
            
            layoutBindings.push_back(descriptorSetLayoutBinding);
            
            binding++;
        }
        
        if(!layoutBindings.empty()) {
            VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {};
            descriptorSetLayoutInfo.bindingCount = layoutBindings.size();
            descriptorSetLayoutInfo.flags = 0;
            descriptorSetLayoutInfo.pBindings = layoutBindings.data();
            descriptorSetLayoutInfo.pNext = nullptr;
            descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            VkResult result = VkFunc::vkCreateDescriptorSetLayout(((VKDevice*)_params._device)->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);
            
            if (result != VK_SUCCESS) {
                assert(0);
                return {};
            }
            
            descriptorSetLayouts.push_back(descriptorSetLayout);
        }
    }
    
    return descriptorSetLayouts;
}
