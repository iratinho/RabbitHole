#include "Renderer/Vendor/WebGPU/WebGPUPipeline.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUCommandBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUShader.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/GPUDefinitions.h"

//https://github.com/pygfx/wgpu-py/pull/574/files#diff-fda84a3fd79c53f5a515591601de9f4e5027e1e9e2e07b84341dc7a7df17f956

void WebGPUGraphicsPipeline::Compile() {
    if(_bWasCompiled) {
        return;
    }
    
    auto device = reinterpret_cast<WebGPUDevice*>(_params._device);
    if(!device) {
        std::cerr << "WebGPUGraphicsPipeline::Compile: device is null" << std::endl;
        return;
    }
    
    WGPUDevice wgpuDevice = device->GetWebGPUDevice();
    if(!wgpuDevice) {
        std::cerr << "WebGPUGraphicsPipeline::Compile: wgpuDevice is null" << std::endl;
        return;
    }
    
//    BuildShaders();

    CompileShaders();
    
    WebGPUShader* vShader = (WebGPUShader*)(_vertexShader.get());
    WebGPUShader* fShader = (WebGPUShader*)(_fragmentShader.get());
    
    if(!vShader || !fShader) {
        return;
    }
    
    const auto& shaderInputBindings = _params._renderPass->CollectShaderInputBindings();
    std::vector<WGPUVertexBufferLayout> vertexBufferLayouts;
    std::vector<WGPUVertexAttribute> vertexAttributes;

    for(auto& [key, value] : shaderInputBindings) {
        int i = 0;
        for(const auto& location : value) {
            WGPUVertexAttribute vertexAttribute {};
            vertexAttribute.format = TranslateVertexFormat(location._format);
            vertexAttribute.offset = location._offset;
            vertexAttribute.shaderLocation = i;

            vertexAttributes.push_back(vertexAttribute);
            i++;
        }

        WGPUVertexBufferLayout bufferLayout;
        bufferLayout.arrayStride = key._stride;
        bufferLayout.stepMode = WGPUVertexStepMode::WGPUVertexStepMode_Vertex;
        bufferLayout.attributeCount = vertexAttributes.size();
        bufferLayout.attributes = vertexAttributes.data();

        vertexBufferLayouts.push_back(bufferLayout);
    }

    WGPURenderPipelineDescriptor  pipelineDesc {};
    pipelineDesc.vertex.bufferCount = vertexBufferLayouts.size();
    pipelineDesc.vertex.buffers = vertexBufferLayouts.data();
    pipelineDesc.vertex.module = vShader->GetWebGPUShaderModule(); // TODO
    pipelineDesc.vertex.entryPoint = "vert_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;
        
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology::WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat::WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = TranslateWindingOrder(_params._rasterization._triangleWindingOrder);
    pipelineDesc.primitive.cullMode = TranslateCullMode(_params._rasterization._triangleCullMode);
    
    const ColorAttachmentBinding& colorAttachmentBinding = _params._renderAttachments._colorAttachmentBinding;
    const std::shared_ptr<Texture2D> colorTexture = colorAttachmentBinding._texture;
    
    if(!colorTexture) {
        assert(false);
        return;
    }
    
    WGPUBlendState colorBlendState {};
    colorBlendState.color.srcFactor = TranslateBlendFactor(colorAttachmentBinding._blending._colorBlendingFactor._srcBlendFactor);
    colorBlendState.color.dstFactor = TranslateBlendFactor(colorAttachmentBinding._blending._colorBlendingFactor._dstBlendFactor);
    colorBlendState.color.operation = TranslateBlendOperation(colorAttachmentBinding._blending._colorBlending);
    colorBlendState.alpha.srcFactor = TranslateBlendFactor(colorAttachmentBinding._blending._alphaBlendingFactor._srcBlendFactor);
    colorBlendState.alpha.dstFactor = TranslateBlendFactor(colorAttachmentBinding._blending._alphaBlendingFactor._dstBlendFactor);
    colorBlendState.alpha.operation = TranslateBlendOperation(colorAttachmentBinding._blending._alphaBlending);
    
    WGPUColorTargetState colorTarget {};
    colorTarget.format = TranslateFormat(colorTexture->GetPixelFormat());
    colorTarget.blend = &colorBlendState;
    colorTarget.writeMask = WGPUColorWriteMask::WGPUColorWriteMask_All;
    
    WGPUFragmentState fragmentState {};
    fragmentState.module = fShader->GetWebGPUShaderModule();
    fragmentState.entryPoint = "frag_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    const std::optional<DepthStencilAttachmentBinding>& depthAttachmentBinding = _params._renderAttachments._depthStencilAttachmentBinding;
    if(depthAttachmentBinding.has_value()) {
        const std::shared_ptr<Texture2D>& depthTexture = depthAttachmentBinding->_texture;
        
        if(!depthTexture) {
            assert(false);
            return;
        }
        
        WGPUStencilFaceState stencilFaceState = {
            .compare = WGPUCompareFunction_Always,   // Default to always pass stencil test
            .failOp = WGPUStencilOperation_Keep,     // Keep stencil value if the stencil test fails
            .depthFailOp = WGPUStencilOperation_Keep, // Keep stencil value if depth test fails
            .passOp = WGPUStencilOperation_Keep      // Keep stencil value if both tests pass
        };
        
        WGPUDepthStencilState depthStencilState {};
        depthStencilState.format = TranslateFormat(depthTexture->GetPixelFormat());
        depthStencilState.depthBias = _params._rasterization._depthBias;
        depthStencilState.depthBiasClamp = _params._rasterization._depthBiasClamp;
        depthStencilState.depthBiasSlopeScale = _params._rasterization._depthBiasSlope;
        depthStencilState.depthCompare = TranslateCompareOP(_params._rasterization._depthCompareOP);
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.stencilFront = stencilFaceState;
        depthStencilState.stencilBack = stencilFaceState;
        
        pipelineDesc.depthStencil = &depthStencilState;
    }
    
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
        
    const std::vector<ShaderDataStream>& dataStreams = _params._renderPass->CollectShaderDataStreams();
    for(const ShaderDataStream& dataStream : dataStreams) {
        if(dataStream._usage != ShaderDataStreamUsage::DATA) {
            continue;
        }

        std::vector<WGPUBindGroupLayoutEntry> groupLayoutEntries;
        
        unsigned int binding = 0;
        for(const ShaderDataBlock& dataBlock : dataStream._dataBlocks) {
            WGPUBindGroupLayoutEntry bindingLayout = {};
            bindingLayout.binding = binding;
            bindingLayout.visibility = TranslateShaderStage(dataBlock._stage);
            
            if (dataBlock._usage == ShaderDataBlockUsage::UNIFORM_BUFFER) {
                // Handle Uniform Buffers
                bindingLayout.buffer.type = WGPUBufferBindingType_Uniform;
                bindingLayout.buffer.minBindingSize = dataBlock._size;
            }
            else if (dataBlock._usage == ShaderDataBlockUsage::TEXTURE) {
                // Handle Textures
                bindingLayout.texture.sampleType = WGPUTextureSampleType_Float; // Or other types as needed
                bindingLayout.texture.viewDimension = WGPUTextureViewDimension_2D;
                bindingLayout.texture.multisampled = false;
            }
            else if (dataBlock._usage == ShaderDataBlockUsage::SAMPLER) {
                bindingLayout.sampler.type = WGPUSamplerBindingType_Filtering; // Or other sampler types
            }

            groupLayoutEntries.push_back(bindingLayout);
            binding++;
        }
        
        WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc{};
        bindGroupLayoutDesc.entryCount = groupLayoutEntries.size();
        bindGroupLayoutDesc.entries = groupLayoutEntries.data();
        
        WGPUBindGroupLayout bindGroupLayout =  wgpuDeviceCreateBindGroupLayout(wgpuDevice, &bindGroupLayoutDesc);
        _bindingGroupLayouts.push_back(bindGroupLayout);
    }

    WGPUPipelineLayoutDescriptor layoutDesc {};
    layoutDesc.bindGroupLayoutCount = _bindingGroupLayouts.size();
    layoutDesc.bindGroupLayouts = _bindingGroupLayouts.data();
    
    _layout = wgpuDeviceCreatePipelineLayout(wgpuDevice, &layoutDesc);
    pipelineDesc.layout = _layout;
    
    _pipeline = wgpuDeviceCreateRenderPipeline(wgpuDevice, &pipelineDesc);
    
    _bWasCompiled = true;
}
