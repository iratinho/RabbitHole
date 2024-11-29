#include "Renderer/Vendor/WebGPU/WebGPUBlitCommandEncoder.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUTextureBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUCommandEncoderSync.hpp"
#include "Renderer/Texture2D.hpp"

WebGPUBlitCommandEncoder::WebGPUBlitCommandEncoder(CommandBuffer* commandBuffer, GraphicsContext* graphicsContext, Device* device)
    : BlitCommandEncoder(commandBuffer, graphicsContext, device) {
    auto wgpuDevice = reinterpret_cast<WebGPUDevice *>(device);
    if(wgpuDevice == nullptr) {
        assert(0 &&  "WebGPUDevice is null");
        return;
    }

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "WebGPUBlitCommandEncoder";
    _encoder = wgpuDeviceCreateCommandEncoder(wgpuDevice->GetWebGPUDevice(), &encoderDesc);
    // std::cout << "wgpuDeviceCreateCommandEncoder (BLIT)" << std::endl;
}

WebGPUBlitCommandEncoder::~WebGPUBlitCommandEncoder() {
    wgpuCommandEncoderRelease(_encoder);
    // std::cout << "wgpuCommandEncoderRelease (BLIT)" << std::endl;
}

void WebGPUBlitCommandEncoder::BeginBlitPass() {
    // std::cout << "_____ BLIT PASS BEING _____" << std::endl;
    WebGPUCommandEncoderSync::GetInstance().SyncCommandEncoder(this);
};

void WebGPUBlitCommandEncoder::EndBlitPass() {
    // std::cout << "_____ BLIT PASS END _____" << std::endl;
};

void WebGPUBlitCommandEncoder::UploadBuffer(std::shared_ptr<Buffer> buffer) {
    WebGPUBuffer* wgpuBuffer = dynamic_cast<WebGPUBuffer*>(buffer.get());
    if(wgpuBuffer) {
        
        WGPUBuffer hostBuffer = wgpuBuffer->GetHostBuffer();
        WGPUBuffer localBuffer = wgpuBuffer->GetLocalBuffer();
        
        if(!hostBuffer || !localBuffer) {
            assert(false && "Trying to upload invalid buffers");
            return;
        }

        wgpuCommandEncoderCopyBufferToBuffer(_encoder, hostBuffer, 0, localBuffer, 0, buffer->GetSize());
        // std::cout << "wgpuCommandEncoderCopyBufferToBuffer (BLIT)" << std::endl;

        return;
    }
    
    assert(false);

};

void WebGPUBlitCommandEncoder::UploadImageBuffer(std::shared_ptr<Texture2D> texture) {
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
        // std::cout << "wgpuCommandEncoderCopyBufferToTexture (BLIT)" << std::endl;
        texture->ClearDirty();
        return;
    }
    
    assert(false);
};

void WebGPUBlitCommandEncoder::CopyImageToImage(const std::shared_ptr<Texture2D>& src, const std::shared_ptr<Texture2D>& dst) {
    auto srcResource = std::static_pointer_cast<WebGPUTextureResource>(src->GetResource());
    auto dstResource = std::static_pointer_cast<WebGPUTextureResource>(dst->GetResource());
    
    WGPUTexture srcTexture = srcResource ? srcResource->GetWGPUTexture() : nullptr;
    WGPUTexture dstTexture = dstResource ? dstResource->GetWGPUTexture() : nullptr;
    
    WGPUImageCopyTexture srcCopy {};
    srcCopy.texture = srcTexture;
    srcCopy.origin.x = 0;
    srcCopy.origin.y = 0;
    
    WGPUImageCopyTexture dstCopy {};
    dstCopy.texture = dstTexture;
    dstCopy.origin.x = 0;
    dstCopy.origin.y = 0;
    
    WGPUExtent3D extent;
    extent.width = dst->GetWidth();
    extent.height = dst->GetHeight();
    extent.depthOrArrayLayers = 1;

    wgpuCommandEncoderCopyTextureToTexture(_encoder, &srcCopy, &dstCopy, &extent);
}
