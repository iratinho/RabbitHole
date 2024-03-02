#include "Renderer/Vendor/Vulkan/VKGraphicsPipeline.hpp"
#include "Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
#include "Renderer/Vendor/Vulkan/VKShader.hpp"
#include "Renderer/Vendor/Vulkan/VkTextureView.hpp"
#include "Renderer/TextureResource.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanLoader.hpp"
#include "Renderer/RenderTarget.hpp"

// TODO REMOVE THIS ASAP, JUST HERE FOR UTILITIES

#include "glm/ext.hpp"

// TODO
// 1 - Pass the image view from render target to image
// 2 - RenderTargets should only be used for attachments
// 3 - Textures should be used to samplers and should contain enough information to derive a VkSampler
// 4 - We should have proper abstraction for render targets and textures
// 5 - Cache the descriptor resources to destroy after pipeline creation
// 6 - Cache descriptor set to later use when drawing
// 7 - Finish function for create framebuffer

//void VKGraphicsPipeline::SetShader(ShaderStage shaderStage, std::unique_ptr<Shader>&& shader) {
//    auto memberPtr = shaderStage == STAGE_VERTEX ? &VKGraphicsPipeline::_vertexShader : &VKGraphicsPipeline::_fragmentShader;
//    this->*memberPtr = shader;
//};

void VKGraphicsPipeline::SetVertexInputs(const std::vector<ShaderInputGroup> &inputGroup) {
  std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptors;
  std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;

  for (const ShaderInputGroup &groupDescriptor : inputGroup) {
    VkVertexInputBindingDescription vertexInputBindingDescription{};
    vertexInputBindingDescription.binding = groupDescriptor._binding;
    vertexInputBindingDescription.stride = groupDescriptor._stride;
    vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    inputBindingDescriptions.push_back(vertexInputBindingDescription);

    for (const ShaderInput &inputDescriptor : groupDescriptor._inputDescriptors) {
      VkVertexInputAttributeDescription vertexInputAttributeDescription{};
      vertexInputAttributeDescription.binding = groupDescriptor._binding;
      vertexInputAttributeDescription.location = inputDescriptor._location;
      vertexInputAttributeDescription.offset = inputDescriptor._memberOffset;
      vertexInputAttributeDescription.format = TranslateFormat(inputDescriptor._format);
      inputAttributeDescriptors.push_back(vertexInputAttributeDescription);
    }
  }
}

void VKGraphicsPipeline::SetVertexInput(const ShaderInputBinding& binding, const ShaderInputLocation& location) {
    _shaderInputs[binding].push_back(location);
}

void VKGraphicsPipeline::SetRasterizationParams(const RasterizationConfiguration &rasterizationConfiguration) {
  _rasterizationConfiguration = rasterizationConfiguration;
}

void VKGraphicsPipeline::DeclareSampler(std::shared_ptr<Texture2D> textureSampler) {
  _textureSamplers.push_back(textureSampler);
}

void VKGraphicsPipeline::Compile() {
    if(_bWasCompiled)
        return;
    
    // Samplers, uniform buffers...
    std::vector<VkDescriptorSetLayout> descriptorLayouts;
    if(CreateDescriptorsSets(descriptorLayouts) != VK_SUCCESS) {
        assert(0);
        return;
    }
    
    std::shared_ptr<VKShader> vShader = std::static_pointer_cast<VKShader>(_params._vertexShader);
    std::shared_ptr<VKShader> fShader = std::static_pointer_cast<VKShader>(_params._fragmentShader);
    
    if(!vShader || !fShader) {
        return;
    }
    
    
    if(!fShader->Compile()) {
        return;
    }
    
    if(!vShader->Compile()) {
        return;
    }
    
    // Push constants
    auto& vertexRange = vShader->GetVertexConstantRange();
    auto& fragmentRange = fShader->GetFragmentConstantRange();
    std::vector<VkPushConstantRange> pushConstantRanges;
    
    if(vertexRange.has_value()) {
        pushConstantRanges.push_back(vertexRange.value());
    }
    
    if(fragmentRange.has_value()) {
        pushConstantRanges.push_back(fragmentRange.value());
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<unsigned int>(pushConstantRanges.size());
    pipelineLayoutCreateInfo.pSetLayouts = descriptorLayouts.data();
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<unsigned int>(descriptorLayouts.size());
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;
    
    VkPipelineLayout pipelineLayout;
    VkResult result = VkFunc::vkCreatePipelineLayout(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    
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
    pipelineRasterizationStateCreateInfo.cullMode = TranslateCullMode(_rasterizationConfiguration._triangleCullMode);
    pipelineRasterizationStateCreateInfo.frontFace = TranslateWindingOrder(_rasterizationConfiguration._triangleWindingOrder);
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.pNext = nullptr;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.depthBiasClamp = _rasterizationConfiguration._depthBiasClamp;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = _rasterizationConfiguration._depthBias;
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = _rasterizationConfiguration._depthBiasSlope;
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
    pipelineDepthStencilStateCreateInfo.depthCompareOp = TranslateCompareOP(_rasterizationConfiguration._depthCompareOP);
    pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;

    VkRenderPass renderPass = CreateRenderPass();

    if (result != VK_SUCCESS) {
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
    
    const bool hasVertexInput = vShader->GetVertexInputInfo().has_value();
    if(!hasVertexInput || shaderStageInfos.size() != 2) {
        return;
    }
    
    VkPipelineVertexInputStateCreateInfo vertexInputState = vShader->GetVertexInputInfo().value();
            
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
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
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multiSampleCreateInfo;
    
    uint32_t pipelineCount = 1;
    VkPipeline pipeline;
    result = VkFunc::vkCreateGraphicsPipelines(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), nullptr, pipelineCount, &graphicsPipelineCreateInfo, nullptr, &pipeline);

    if (result != VK_SUCCESS) {
        assert(0);
        return;
    }
    
    _bWasCompiled = true;
}

void VKGraphicsPipeline::Draw(const PrimitiveProxy& proxy) {
//    if(!_onDrawCallback(proxy)) {
//        return;
//    }
    
    // VULKAN DRAWING
}

VkResult VKGraphicsPipeline::CreateDescriptorsSets(std::vector<VkDescriptorSetLayout>&  descriptorLayouts) {
    VKGraphicsContext* context = (VKGraphicsContext*)_params._graphicsContext;
    if(!context) {
        return VK_ERROR_UNKNOWN;
    }
    
    // TODO materials could be responsible to create the descriptor sets, since its guaranteed that the shader needs to support all samplers
    if (_textureSamplers.size() > 0) {
        VkDescriptorSetLayoutBinding imageDescLayoutBinding{};
        imageDescLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescLayoutBinding.binding = 0;
        imageDescLayoutBinding.descriptorCount = 1;
        imageDescLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = 1;
        descriptorSetLayoutInfo.pBindings = &imageDescLayoutBinding;

        VkDescriptorSetLayout descriptorSetLayout;
        VkResult result = VkFunc::vkCreateDescriptorSetLayout(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &descriptorSetLayoutInfo,nullptr, &descriptorSetLayout);

        if (result != VK_SUCCESS) {
            return result;
        }

        descriptorLayouts.push_back(descriptorSetLayout);

        std::vector<VkDescriptorImageInfo> descriptorImageInfos;
        for (auto &texture : _textureSamplers) {
            // TODO Move to texture
            VkSamplerCreateInfo samplerCreateInfo{};
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
            VkResult result = VkFunc::vkCreateSampler(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &samplerCreateInfo, nullptr, &sampler);
            
            if (result != VK_SUCCESS) {
                return result;
            }
            
            Range range (0, 1);
            auto view = texture->MakeTextureView(texture->GetPixelFormat(), range);
            const auto vkView = (VkTextureView*)view;
            
            VkDescriptorImageInfo descriptorImageInfo{};
            descriptorImageInfo.imageView = (VkImageView)vkView->GetImageView();
            descriptorImageInfo.sampler = sampler;
            descriptorImageInfos.push_back(descriptorImageInfo);
        }
        
        // TODO this should be one descriptor set per in flight frame...
        
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = context->GetDescriptorPool();
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        VkDescriptorSet descriptorSet;
        result = VkFunc::vkAllocateDescriptorSets(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &allocInfo, &descriptorSet);
        if (result != VK_SUCCESS) {
            return result;
        }

        // This type of updates should be part of rendering, meaning, that we should vkUpdateDescriptorSets when resources change
        // Lets say that a VKImage data as changed, we need to update the descriptor sets. We need to have a system to indentify that
        // a resource was dirty and update it... Or we want to change the sampler at runtime
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.pImageInfo = descriptorImageInfos.data();
        writeDescriptorSet.descriptorCount = static_cast<unsigned int>(descriptorImageInfos.size());
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.dstBinding = imageDescLayoutBinding.binding;
        writeDescriptorSet.pBufferInfo = nullptr;

        VkFunc::vkUpdateDescriptorSets(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), 1, &writeDescriptorSet, 0, nullptr);
    }

    return VK_SUCCESS;
}

std::vector<VkPipelineColorBlendAttachmentState> VKGraphicsPipeline::CreateColorBlendAttachemnt() {
    std::vector<VkPipelineColorBlendAttachmentState> colorAttachmentStates;
    
    Attachment& attachment = _params._colorAttachment;
    
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = attachment.bBlendEnabled;
    
    const Ops::BlendFactorOps& blendFactorOps = attachment._blendFactorOp.GetValue<Ops::BLEND_FACTOR>();
    colorBlendAttachmentState.srcColorBlendFactor = TranslateBlendFactor(blendFactorOps.srcFactor);
    colorBlendAttachmentState.dstColorBlendFactor = TranslateBlendFactor(blendFactorOps.destFactor);
    colorBlendAttachmentState.srcAlphaBlendFactor = TranslateBlendFactor(blendFactorOps.alphaSrcFactor);
    colorBlendAttachmentState.dstAlphaBlendFactor = TranslateBlendFactor(blendFactorOps.alphaDestFactor);

    const Ops::BlendOps& blendOps = attachment._blendOp.GetValue<Ops::BLEND>();
    colorBlendAttachmentState.colorBlendOp = TranslateBlendOperation(blendOps.colorOp);
    colorBlendAttachmentState.alphaBlendOp = TranslateBlendOperation(blendOps.alphaOp);
    
    colorAttachmentStates.push_back(colorBlendAttachmentState);

    return colorAttachmentStates;
}

std::vector<VkAttachmentDescription> VKGraphicsPipeline::CreateAttachmentDescriptions() {
    std::vector<VkAttachmentDescription> attachmentDescriptors;

    VkAttachmentDescription colorAttachmentDesc {};
    VkAttachmentDescription depthAttachmentDesc {};
    
    // Color attachment
    {
        Attachment& colorAttachment = _params._colorAttachment;
        
        const Ops::LayoutOp& layoutOp = colorAttachment._layoutOp.GetValue<Ops::LAYOUT>();
        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //TranslateImageLayout(layoutOp._oldLayout);
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //TranslateImageLayout(layoutOp._newLayout);
        
        const Ops::LoadStoreOps& loadStoreOps = colorAttachment._loadStoreOp.GetValue<Ops::LOAD_STORE>();
        colorAttachmentDesc.loadOp = TranslateLoadOP(loadStoreOps.load);
        colorAttachmentDesc.storeOp = TranslateStoreOP(loadStoreOps.store);
        colorAttachmentDesc.stencilLoadOp = TranslateLoadOP(loadStoreOps.stencilLoad);
        colorAttachmentDesc.stencilStoreOp = TranslateStoreOP(loadStoreOps.stencilStore);
        
        colorAttachmentDesc.format = TranslateFormat(colorAttachment._format);
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDesc.flags = 0;
    }
    
    // Depth attachment
    {
        Attachment& depthAttachment = _params._depthAttachment;

        const Ops::LayoutOp& layoutOp = depthAttachment._layoutOp.GetValue<Ops::LAYOUT>();
        depthAttachmentDesc.initialLayout = TranslateImageLayout(layoutOp._oldLayout);
        depthAttachmentDesc.finalLayout = TranslateImageLayout(layoutOp._newLayout);

        const Ops::LoadStoreOps& loadStoreOps = depthAttachment._loadStoreOp.GetValue<Ops::LOAD_STORE>();
        depthAttachmentDesc.loadOp = TranslateLoadOP(loadStoreOps.load);
        depthAttachmentDesc.storeOp = TranslateStoreOP(loadStoreOps.store);
        depthAttachmentDesc.stencilLoadOp = TranslateLoadOP(loadStoreOps.stencilLoad);
        depthAttachmentDesc.stencilStoreOp = TranslateStoreOP(loadStoreOps.stencilStore);
        depthAttachmentDesc.format = TranslateFormat(depthAttachment._format);
        depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachmentDesc.flags = 0;
    }

    attachmentDescriptors.push_back(colorAttachmentDesc);
    attachmentDescriptors.push_back(depthAttachmentDesc);
    
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
    subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.flags = 0;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Depth
    subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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

    VkResult result = VkFunc::vkCreateRenderPass(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &renderPassCreateInfo, nullptr, &_renderPass);

    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    return _renderPass;
}

// Should render target be resposible for storing the mips levels and have its own TextureView?
/*
 * This also means i need to support creating render targets from textures, right now i only support render targets that auto create textures.
 * What if i want to have multiple render targets that point to the same texture but at different levels or formats?
 */
VkFramebuffer VKGraphicsPipeline::CreateFrameBuffer(std::vector<RenderTarget*> renderTargets) {
    uint32_t width = 0;
    uint32_t height = 0;
    
    std::vector<VkImageView> imageViews;
    for(auto renderTarget : renderTargets) {
        VkTextureView* textureView = (VkTextureView*)renderTarget->GetTexture()->MakeTextureView();
        if(textureView) {
            VkImageView imageView = (VkImageView)textureView->GetImageView();
            if(imageView) {
                imageViews.push_back(imageView);
            }
        }
        
        width = std::max(width, renderTarget->GetTexture()->GetWidth());
        height = std::max(height, renderTarget->GetTexture()->GetHeight());
    }
    
    bool bIsDirty = false;
    
    if(imageViews.size() == 0) {
        return nullptr;
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

    // Most likelly the first time we create a framebuffer with this pipeline
    if(_views.empty()) {
        _views = imageViews;
        if(VkFunc::vkCreateFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &frameBufferInfo, VK_NULL_HANDLE, &_frameBuffer) != VK_SUCCESS) {
            std::cerr << "Unable to create vulkan framebuffer" << std::endl;
            return nullptr;
        }
    }
    
//    // We already had cached image views, lets check if the new ones match with the ones we have
//    // in case it wont match it means that we need to create a new framebuffer
//    if(_views != imageViews) {
//        VkFunc::vkDestroyFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), _frameBuffer, VK_NULL_HANDLE);
//        VkFunc::vkCreateFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), &frameBufferInfo, VK_NULL_HANDLE, &_frameBuffer);
//    }
    
    return _frameBuffer;
}

void VKGraphicsPipeline::DestroyFrameBuffer() {
    VkFunc::vkDestroyFramebuffer(_params._graphicsContext->GetDevice()->GetLogicalDeviceHandle(), _frameBuffer, VK_NULL_HANDLE);
    _views.clear();
}

