#include "Renderer/RenderPass/RenderPassGenerator.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/render_context.hpp"
#include "Renderer/VulkanTranslator.hpp"
#include "Core/Utils.hpp"
#include "Renderer/RenderGraph/RenderGraph.hpp"
#include "Renderer/RenderSystem.hpp"

namespace {
    template<typename ...Args>
    std::string ComputePSOCacheKey(unsigned int frameIndex, const Args &... args) {
        unsigned int hash = hash_value(args...);
        return std::to_string(hash) + "-frame-" + std::to_string(frameIndex);
    }
}

RenderPassGenerator::RenderPassGenerator() {
    _attachments.reserve(10);
    _pushConstants.reserve(10);
    _primitiveData.reserve(10);
}

RasterizationConfiguration &RenderPassGenerator::ConfigureRasterizationOptions() {
    return _rasterizationConfiguration;
}

ShaderConfiguration &RenderPassGenerator::ConfigureShader(ShaderStage shaderStage) {
    return _shaderConfiguration[shaderStage];
}

AttachmentConfiguration &RenderPassGenerator::MakeAttachment() {
    _attachments.emplace_back();
    return _attachments.back();
}

PushConstantConfiguration &RenderPassGenerator::MakePushConstant() {
    _pushConstants.emplace_back();
    return _pushConstants.back();
}

InputGroupDescriptor &RenderPassGenerator::MakeInputGroupDescriptor() {
    _inputGroupDescriptors.emplace_back();
    return _inputGroupDescriptors.back();
}

PrimitiveProxy &RenderPassGenerator::MakePrimitiveProxy() {
    _primitiveData.emplace_back();
    return _primitiveData.back();
}

PipelineStateObject *RenderPassGenerator::Generate(RenderContext *renderContext, unsigned int frameIndex) {
    RenderGraph *renderGraph = renderContext->GetRenderSystem()->GetRenderGraph();

    // TODO big hazzard, this is not enough info to have unique key....
    // Compute hash for values that impact the pipeline creation
    const std::string psoKey = ComputePSOCacheKey(frameIndex, _rasterizationConfiguration);
    if (renderGraph->GetCachedPSO3(psoKey)) {
        return renderGraph->GetCachedPSO3(psoKey);
    }

    // ------- PIPELINE LAYOUT ------- //
    std::vector<VkPushConstantRange> pushConstants;
    unsigned int pushConstantOffset = 0;
    for (const PushConstantConfiguration &pushConstantConfiguration: _pushConstants) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.offset = pushConstantOffset;
        pushConstantRange.size = pushConstantConfiguration._pushConstant._size;
        pushConstantRange.stageFlags = TranslateShaderStage(pushConstantConfiguration._pushConstant._shaderStage);
        pushConstants.emplace_back(pushConstantRange);
        
        pushConstantOffset += pushConstantConfiguration._pushConstant._size;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
    pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstants.size();
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.flags = 0;

    VkPipelineLayout pipelineLayout;
    VkResult result = VkFunc::vkCreatePipelineLayout(renderContext->GetLogicalDeviceHandle(), &pipelineLayoutCreateInfo,
                                                     nullptr, &pipelineLayout);

    if (result != VK_SUCCESS) {
        return nullptr;
    }

    // ------- RENDER PASS ------- //
    // TODO need to handle subpass somehow, we could create groups of subpass with each attachment
    std::vector<VkAttachmentDescription> _attachmentDescriptions;
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> depthAttachments;
    int attachmentCounter = 0;
    bool bFoundDepthAttachment = false;
    for (const AttachmentConfiguration &attachmentConfiguration: _attachments) {
        bool bIsColor = attachmentConfiguration._attachment._format < AttachmentFormat::END_COLOR_FORMATS;
        bFoundDepthAttachment |= !bIsColor;

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.initialLayout = TranslateImageLayout(attachmentConfiguration._initialLayout) ;
        attachmentDescription.finalLayout = TranslateImageLayout(attachmentConfiguration._finalLayout);
        attachmentDescription.loadOp = TranslateLoadOP(attachmentConfiguration._attachment._loadOp);
        attachmentDescription.storeOp = TranslateStoreOP(attachmentConfiguration._attachment._storeOp);
        attachmentDescription.stencilLoadOp = TranslateLoadOP(attachmentConfiguration._attachment._stencilLoadOp);
        attachmentDescription.stencilStoreOp = TranslateStoreOP(attachmentConfiguration._attachment._stencilStoreOp);
        attachmentDescription.format = TranslateFormat(attachmentConfiguration._attachment._format);
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.flags = 0;

        VkAttachmentReference attachmentReference{};
        attachmentReference.attachment = attachmentCounter;
        attachmentReference.layout = TranslateImageLayout(attachmentConfiguration._initialLayout);

        _attachmentDescriptions.emplace_back(attachmentDescription);

        if (bIsColor) {
            std::cout << "ColorAttachmentDescription" << std::endl;
            std::cout << hash_value(attachmentDescription) << std::endl;

            colorAttachments.emplace_back(attachmentReference);
        } else {
            depthAttachments.emplace_back(attachmentReference);
        }

        attachmentCounter++;
    }

    /*
     * NOTE: At the moment we are not able to infer subpass information, for now we assume
     * the most basic setup (1 subpass), but this information should be grouped with attachments
     * for example, a set of attachments belong to a subpass. src and dst masks should also be specified
     * in this group...
     */

    VkSubpassDescription subpassDescription{};
    subpassDescription.pColorAttachments = colorAttachments.data();
    subpassDescription.colorAttachmentCount = colorAttachments.size();
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pDepthStencilAttachment = depthAttachments.data();
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.flags = 0;

    std::cout << "SubpassDescription" << std::endl;
    std::cout << hash_value(subpassDescription) << std::endl;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    subpassDependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


//    if(bFoundDepthAttachment) {
//        subpassDependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//        subpassDependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//        subpassDependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//    }

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pAttachments = _attachmentDescriptions.data();
    renderPassCreateInfo.attachmentCount = _attachmentDescriptions.size();
    renderPassCreateInfo.pDependencies = &subpassDependency;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pNext = nullptr;
    renderPassCreateInfo.flags = 0;

    VkRenderPass renderPass;
    result = VkFunc::vkCreateRenderPass(renderContext->GetLogicalDeviceHandle(), &renderPassCreateInfo, nullptr,
                                        &renderPass);

    if (result != VK_SUCCESS) {
        return nullptr;
    }

    // ------- PIPELINE ------- //
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
    pipelineRasterizationStateCreateInfo.flags = 0;
    pipelineRasterizationStateCreateInfo.cullMode = TranslateCullMode(_rasterizationConfiguration._triangleCullMode);
    pipelineRasterizationStateCreateInfo.frontFace = TranslateWindingOrder(
            _rasterizationConfiguration._triangleWindingOrder);
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.pNext = nullptr;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
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

    std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptors;
    std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;

    for (const InputGroupDescriptor &groupDescriptor: _inputGroupDescriptors) {
        VkVertexInputBindingDescription vertexInputBindingDescription{};
        vertexInputBindingDescription.binding = groupDescriptor._binding;
        vertexInputBindingDescription.stride = groupDescriptor._stride;
        vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindingDescriptions.push_back(vertexInputBindingDescription);

        for (const InputDescriptor &inputDescriptor: groupDescriptor._inputDescriptors) {
            VkVertexInputAttributeDescription vertexInputAttributeDescription{};
            vertexInputAttributeDescription.binding = inputDescriptor._binding;
            vertexInputAttributeDescription.location = inputDescriptor._location;
            vertexInputAttributeDescription.offset = inputDescriptor._memberOffset;
            vertexInputAttributeDescription.format = TranslateFormat(inputDescriptor._format);
            inputAttributeDescriptors.push_back(vertexInputAttributeDescription);
        }
    }

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
    pipelineVertexInputStateCreateInfo.flags = 0;
    pipelineVertexInputStateCreateInfo.pNext = nullptr;
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptors.data();
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributeDescriptors.size();
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = inputBindingDescriptions.size();

    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
    pipelineDepthStencilStateCreateInfo.flags = 0;
    pipelineDepthStencilStateCreateInfo.pNext = nullptr;
    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineDepthStencilStateCreateInfo.depthCompareOp = TranslateCompareOP(_rasterizationConfiguration._depthCompareOP);
    pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
                               
    std::vector<VkPipelineColorBlendAttachmentState> attachmentsBlendStates;
    for (const AttachmentConfiguration &attachmentConfiguration: _attachments) {
        bool bIsColor = attachmentConfiguration._attachment._format < AttachmentFormat::END_COLOR_FORMATS;

        if (bIsColor) {
            // TODO incomplete the information should reside in the attachment configuration
            VkPipelineColorBlendAttachmentState attachmentBlendState;
            attachmentBlendState.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT;
            attachmentBlendState.blendEnable = attachmentConfiguration._attachment.bBlendEnabled;
            attachmentBlendState.srcColorBlendFactor = TranslateBlendFactor(
                    attachmentConfiguration._attachment._srcColorBlendFactor);
            attachmentBlendState.dstColorBlendFactor = TranslateBlendFactor(
                    attachmentConfiguration._attachment._dstColorBlendFactor);
            attachmentBlendState.srcAlphaBlendFactor = TranslateBlendFactor(
                    attachmentConfiguration._attachment._srcAlphaBlendFactor);
            attachmentBlendState.dstAlphaBlendFactor = TranslateBlendFactor(
                    attachmentConfiguration._attachment._dstAlphaBlendFactor);;
            attachmentBlendState.colorBlendOp = TranslateBlendOperation(
                    attachmentConfiguration._attachment._colorBlendOp);
            attachmentBlendState.alphaBlendOp = TranslateBlendOperation(
                    attachmentConfiguration._attachment._alphaBlendOp);
            attachmentsBlendStates.push_back(attachmentBlendState);
        }
    }

    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
    pipelineColorBlendStateCreateInfo.flags = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[0] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[1] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[2] = 0;
    pipelineColorBlendStateCreateInfo.blendConstants[3] = 0;
    pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    pipelineColorBlendStateCreateInfo.pAttachments = attachmentsBlendStates.data();
    pipelineColorBlendStateCreateInfo.attachmentCount = attachmentsBlendStates.size();
    pipelineColorBlendStateCreateInfo.pNext = nullptr;
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
    const bool bCreatedVertexShader = renderContext->CreateShader(
            _shaderConfiguration[ShaderStage::STAGE_VERTEX]._shaderPath, ShaderStage::STAGE_VERTEX, shaderStages[0]);
    const bool bCreatedFragmentShader = renderContext->CreateShader(
            _shaderConfiguration[ShaderStage::STAGE_FRAGMENT]._shaderPath, ShaderStage::STAGE_FRAGMENT,
            shaderStages[1]);

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.flags = 0;
    graphicsPipelineCreateInfo.layout = pipelineLayout;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.pStages = shaderStages.data();
    graphicsPipelineCreateInfo.stageCount = shaderStages.size();
    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;

    uint32_t pipelineCount = 1;
    VkPipeline pipeline;
    result = VkFunc::vkCreateGraphicsPipelines(renderContext->GetLogicalDeviceHandle(), nullptr, pipelineCount,
                                               &graphicsPipelineCreateInfo, nullptr, &pipeline);

    if (result != VK_SUCCESS) {
        return nullptr;
    }

    PipelineStateObject pso;
    pso.render_pass = renderPass;
    pso.pipeline_layout = pipelineLayout;
    pso.pipeline = pipeline;

    return renderGraph->RegisterPSO2(psoKey, pso);
}

void RenderPassGenerator::AddPrimitiveProxy(PrimitiveProxy&& primitiveProxy) {
    _primitiveData.push_back(primitiveProxy);
}





