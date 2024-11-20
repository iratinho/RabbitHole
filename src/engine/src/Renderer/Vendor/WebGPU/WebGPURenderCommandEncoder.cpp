#include "Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.hpp"

#include "Renderer/Texture2D.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureView.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUPipeline.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"

WebGPURenderCommandEncoder::WebGPURenderCommandEncoder(CommandBuffer *commandBuffer, GraphicsContext *graphicsContext,
                                                       Device *device) : RenderCommandEncoder(commandBuffer, graphicsContext, device)
{
    auto wgpuDevice = reinterpret_cast<WebGPUDevice *>(device);
    if(wgpuDevice == nullptr) {
        assert(0 &&  "WebGPUDevice is null");
        return;
    }

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "WebGPURenderCommandEncoder";
    _encoder = wgpuDeviceCreateCommandEncoder(wgpuDevice->GetWebGPUDevice(), &encoderDesc);
    std::cout << "wgpuDeviceCreateCommandEncoder" << std::endl;

    if(_encoder == nullptr) {
        assert(0 &&  "WebGPUCommandEncoder creation failed");
        return;
    }
}

WebGPURenderCommandEncoder::~WebGPURenderCommandEncoder() {
    wgpuCommandEncoderRelease(_encoder);
    std::cout << "wgpuCommandEncoderRelease" << std::endl;    
}

void WebGPURenderCommandEncoder::BeginRenderPass(GraphicsPipeline *pipeline, const RenderAttachments &attachments) {
    
    WebGPUGraphicsPipeline* wgpuPipeline = (WebGPUGraphicsPipeline*)(pipeline);
    if(!wgpuPipeline) {
        assert(false && "Unable to begin render pass, invalid pipeline.");
        return;
    }
    
    Texture2D* colorAttachmentTexture = attachments._colorAttachmentBinding._texture.get();
    auto colorAttachmentView = reinterpret_cast<WebGPUTextureView *>(colorAttachmentTexture->MakeTextureView());

    WGPURenderPassColorAttachment renderPassColorAttachment = {};
    renderPassColorAttachment.view = colorAttachmentView->GetWebGPUTextureView();
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = TranslateLoadOP(attachments._colorAttachmentBinding._loadAction);
    renderPassColorAttachment.storeOp = WGPUStoreOp::WGPUStoreOp_Store;
    
    const float darkness = 0.28f;
    renderPassColorAttachment.clearValue = WGPUColor{ 0.071435f * darkness, 0.079988f * darkness, 0.084369f * darkness, 0.0 };
#ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif

    WGPURenderPassDepthStencilAttachment renderPassDepthAttachment = {};
    if(attachments._depthStencilAttachmentBinding.has_value()) {
        Texture2D* depthAttachmentTexture = attachments._depthStencilAttachmentBinding->_texture.get();
        auto depthAttachmentView = reinterpret_cast<WebGPUTextureView *>(depthAttachmentTexture->MakeTextureView());
        
        renderPassDepthAttachment.view = depthAttachmentView->GetWebGPUTextureView();
        renderPassDepthAttachment.depthClearValue = 1.0f;
        renderPassDepthAttachment.depthLoadOp = TranslateLoadOp( attachments._depthStencilAttachmentBinding->_depthLoadAction);
        // For now assume that every render pass wants to store its results
        renderPassDepthAttachment.depthStoreOp = WGPUStoreOp::WGPUStoreOp_Store;
        renderPassDepthAttachment.depthReadOnly = false;
    }
    
    WGPURenderPassDescriptor renderPassDescriptor = {};
    renderPassDescriptor.nextInChain = nullptr;
    renderPassDescriptor.colorAttachments = &renderPassColorAttachment;
    renderPassDescriptor.colorAttachmentCount = 1;
    renderPassDescriptor.depthStencilAttachment = &renderPassDepthAttachment;
    renderPassDescriptor.label = "MyEEncoder";

    _encoderPass = wgpuCommandEncoderBeginRenderPass(_encoder, &renderPassDescriptor);
    std::cout << "wgpuCommandEncoderBeginRenderPass" << std::endl;

    if(_encoderPass == nullptr) {
        assert(0 &&  "WGPURenderPassEncoder creation failed");
        return;
    }
    
    wgpuRenderPassEncoderSetPipeline(_encoderPass, wgpuPipeline->GetWebGPUPipeline());
        std::cout << "wgpuRenderPassEncoderSetPipeline" << std::endl;
}

void WebGPURenderCommandEncoder::EndRenderPass() {
    wgpuRenderPassEncoderEnd(_encoderPass);
    std::cout << "wgpuRenderPassEncoderEnd" << std::endl;
    wgpuRenderPassEncoderRelease(_encoderPass);
    std::cout << "wgpuRenderPassEncoderRelease" << std::endl;
        
    _encoderPass = nullptr;
}

void WebGPURenderCommandEncoder::SetViewport(const glm::vec2 &viewportSize) {
    if(_encoderPass) {
        wgpuRenderPassEncoderSetViewport(_encoderPass,
            0,
            0,
            viewportSize.x,
            viewportSize.y,
            0,
            1);
        
        std::cout << "wgpuRenderPassEncoderSetViewport" << std::endl;

    }
}

void WebGPURenderCommandEncoder::SetScissor(const glm::vec2 &extent, const glm::vec2 &offset) {
    if(_encoderPass) {
        wgpuRenderPassEncoderSetScissorRect(_encoderPass,
            static_cast<std::uint32_t>(offset.x),
            static_cast<std::uint32_t>(offset.y),
            static_cast<std::uint32_t>(extent.x),
            static_cast<std::uint32_t>(extent.y));
        std::cout << "wgpuRenderPassEncoderSetScissorRect" << std::endl;
    }
}

void WebGPURenderCommandEncoder::DispatchDataStreams(GraphicsPipeline* graphicsPipeline, const std::vector<ShaderDataStream> dataStreams) {
    WebGPUGraphicsPipeline* wgpuPipeline = (WebGPUGraphicsPipeline*)(graphicsPipeline);
    if(!wgpuPipeline) {
        assert(false && "Unable to begin render pass, invalid pipeline.");
        return;
    }
    
    WebGPUDevice* wgpuDevice = (WebGPUDevice*)_graphicsContext->GetDevice();
    if(!wgpuDevice) {
        assert(false && "Unable to begin render pass, invalid device.");
        return;
    }
    
    // TODO Create a manager to create and cache bind groups based on the current data stream
    unsigned int dataStreamCount = 0;
    for(const ShaderDataStream& dataStream : dataStreams) {
        
        std::vector<WGPUBindGroupEntry> groupEntries;
        
        unsigned int binding = 0;
        for(const ShaderDataBlock& dataBlock : dataStream._dataBlocks) {
            WGPUBindGroupEntry bindGroupEntry {};
            bindGroupEntry.size = dataBlock._size;
            bindGroupEntry.offset = 0;
            bindGroupEntry.binding = binding;

            if(dataBlock._usage == ShaderDataBlockUsage::UNIFORM_BUFFER) {
                if(!std::holds_alternative<ShaderBufferResource>(dataBlock._data)) {
                    assert(false);
                    continue;
                }
                
                const ShaderBufferResource& bsr = std::get<ShaderBufferResource>(dataBlock._data);
                
                WebGPUBuffer* wgpuBuffer = (WebGPUBuffer*)bsr._bufferResource.get();
                if(!wgpuBuffer) {
                    assert(false);
                    continue;
                }
                
                // We did not upload the buffer to the gpu, do it now
                UploadBuffer(bsr._bufferResource);
                
                bindGroupEntry.buffer = wgpuBuffer->GetLocalBuffer();
                bindGroupEntry.offset = bsr._offset;
            }
            
            if(dataBlock._usage == ShaderDataBlockUsage::TEXTURE) {
                if(!std::holds_alternative<ShaderTextureResource>(dataBlock._data)) {
                    assert(false);
                    continue;
                }
                
                const ShaderTextureResource& tr = std::get<ShaderTextureResource>(dataBlock._data);
                
                WebGPUTextureView* textureView = (WebGPUTextureView*)tr._texture->MakeTextureView();
                if(!textureView) {
                    assert(false);
                    continue;
                }
                
                bindGroupEntry.textureView = textureView->GetWebGPUTextureView();
            }
            
            if(dataBlock._usage == ShaderDataBlockUsage::SAMPLER) {
                if(!std::holds_alternative<ShaderTextureResource>(dataBlock._data)) {
                    assert(false);
                    continue;
                }
                
                const ShaderTextureResource& tr = std::get<ShaderTextureResource>(dataBlock._data);
                
                WGPUSamplerDescriptor samplerDescriptor {};
                samplerDescriptor.addressModeU = TranslateWrapMode(tr._sampler._wrapU);
                samplerDescriptor.addressModeV = TranslateWrapMode(tr._sampler._wrapV);
                samplerDescriptor.addressModeW = TranslateWrapMode(tr._sampler._wrapW);
                samplerDescriptor.magFilter = TranslateTextureFilter(tr._sampler._magFilter);
                samplerDescriptor.minFilter = TranslateTextureFilter(tr._sampler._minFilter);
                samplerDescriptor.maxAnisotropy = 1;
                
                // TODO avoid creating samplers all the time, can we do the same as vulkan?
                WGPUSampler sampler = wgpuDeviceCreateSampler(wgpuDevice->GetWebGPUDevice(), &samplerDescriptor);
                std::cout << "wgpuDeviceCreateSampler" << std::endl;
                bindGroupEntry.sampler = sampler;
            }
            
            groupEntries.push_back(bindGroupEntry);

            binding++;
        }
        
        const std::vector<WGPUBindGroupLayout>& layouts = wgpuPipeline->GetWebGPUGroupLayouts();
        
        WGPUBindGroupDescriptor bindGroupDescriptor {};
        bindGroupDescriptor.label = "";
        bindGroupDescriptor.entryCount = groupEntries.size();
        bindGroupDescriptor.entries = groupEntries.data();
        bindGroupDescriptor.layout = layouts[dataStreamCount];
  
        // TODO can we cache bind groups like in vulkan where we cache descriptors?
        WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(wgpuDevice->GetWebGPUDevice(), &bindGroupDescriptor);
        std::cout << "wgpuDeviceCreateBindGroup" << std::endl;

        wgpuRenderPassEncoderSetBindGroup(_encoderPass, dataStreamCount, bindGroup, 0, nullptr);
        std::cout << "wgpuRenderPassEncoderSetBindGroup" << std::endl;

//        WGPUBindGroup bindGroup = wgpuDeviceCreateBindGroup(wgpuDevice->GetWebGPUDevice(), &bindGroupDescriptor);
//        
//        
//        wgpuRenderPassEncoderSetBindGroup(_encoderPass, 0, ,0, bindGroup, 0, nullptr);
        
        dataStreamCount++;
    }
    
}

void WebGPURenderCommandEncoder::DrawPrimitiveIndexed(const PrimitiveProxyComponent &proxy) {
    if(_encoderPass) {
        // TODO
        
        WebGPUBuffer* wgpuBuffer = (WebGPUBuffer*)proxy._gpuBuffer.get();
        if(!wgpuBuffer) {
            assert(false && "Invalid vertex buffer");
            return;
        }
        
        wgpuRenderPassEncoderSetIndexBuffer(_encoderPass, wgpuBuffer->GetLocalBuffer(), WGPUIndexFormat_Uint32, proxy._indicesOffset, WGPU_WHOLE_SIZE);
        std::cout << "wgpuRenderPassEncoderSetIndexBuffer" << std::endl;

        wgpuRenderPassEncoderSetVertexBuffer(_encoderPass, 0, wgpuBuffer->GetLocalBuffer(), proxy._vertexOffset, WGPU_WHOLE_SIZE);
        std::cout << "wgpuRenderPassEncoderSetVertexBuffer" << std::endl;

        wgpuRenderPassEncoderDrawIndexed(_encoderPass, proxy._indicesCount, 1, 0, 0, 0);
        std::cout << "wgpuRenderPassEncoderDrawIndexed" << std::endl;
    }
}

void WebGPURenderCommandEncoder::Draw(std::uint32_t count) {
    if(_encoderPass) {
        wgpuRenderPassEncoderDraw(_encoderPass, count, 1, 0, 0);
        std::cout << "wgpuRenderPassEncoderDraw" << std::endl;
    }
}

void WebGPURenderCommandEncoder::MakeImageBarrier(Texture2D *texture2D, ImageLayout after) {
    // No implementation for barriers
}

void WebGPURenderCommandEncoder::UploadBuffer(std::shared_ptr<Buffer> buffer) {
    WebGPUBuffer* wgpuBuffer = dynamic_cast<WebGPUBuffer*>(buffer.get());
    if(wgpuBuffer) {
        
        hostBuffer = wgpuBuffer->GetHostBuffer();
        localBuffer = wgpuBuffer->GetLocalBuffer();
        
        if(!hostBuffer || !localBuffer) {
            assert(false && "Trying to upload invalid buffers");
            return;
        }

        // EM_ASM_({
        //     const encoder = WebGPU.mgrCommandEncoder.get($0);
        //     const src = WebGPU.mgrBuffer.get($1);
        //     const dst = WebGPU.mgrBuffer.get($2);
        //     encoder.copyBufferToBuffer(src, $3, dst, $4, $5);
        // }, _encoder, hostBuffer, localBuffer, 0, 0, buffer->GetSize());


        // Manually invoke the JS function using EM_ASM_ for some reason the binding is not working, INVESTIGATE!!
        // EM_ASM({
        //     const encoder = WebGPU.mgrCommandEncoder.get($0);
        //     const src = WebGPU.mgrBuffer.get($1);
        //     const dst = WebGPU.mgrBuffer.get($3);
        //     encoder.copyBufferToBuffer(src, $2, dst, $4, $5);
        // }, _encoder, hostBuffer, 0, localBuffer, 0, buffer->GetSize());

        wgpuCommandEncoderCopyBufferToBuffer(_encoder, hostBuffer, 0, localBuffer, 0, buffer->GetSize());
        std::cout << "wgpuCommandEncoderCopyBufferToBuffer" << std::endl;

        return;
    }
    
    assert(false);
}

void WebGPURenderCommandEncoder::UploadImageBuffer(std::shared_ptr<Texture2D> texture) {
    WebGPUTextureResource* textureResource = texture != nullptr ? (WebGPUTextureResource*)texture->GetResource().get() : nullptr;
    if(!textureResource) {
        assert(false && "Trying to upload invalid texture");
        return;
    }

    WebGPUTextureBuffer* textureBuffer = textureResource ? (WebGPUTextureBuffer*)textureResource->GetBuffer().get() : nullptr;
    
    if(textureBuffer) {
        WGPUBuffer hostBuffer = textureBuffer->GetHostBuffer();
        
        if(!hostBuffer) {
            assert(false && "Trying to upload invalid buffers");
            return;
        }
        
        WGPUImageCopyBuffer imageCopyBuffer {};
        imageCopyBuffer.buffer = hostBuffer;
        imageCopyBuffer.layout.bytesPerRow = texture->GetWidth() * sizeof(float);
        imageCopyBuffer.layout.rowsPerImage = texture->GetWidth();
        imageCopyBuffer.layout.offset = 0;
        
        WGPUImageCopyTexture imageCopyTexture {};
        imageCopyTexture.texture = textureResource->GetWGPUTexture();
        
        WGPUExtent3D extent {};
        extent.width = texture->GetWidth();
        extent.height = texture->GetHeight();
        extent.depthOrArrayLayers = 1;
        
        wgpuCommandEncoderCopyBufferToTexture(_encoder, &imageCopyBuffer, &imageCopyTexture, &extent);
        std::cout << "wgpuCommandEncoderCopyBufferToTexture" << std::endl;
        texture->ClearDirty();
        return;
    }
    
    assert(false);
}

