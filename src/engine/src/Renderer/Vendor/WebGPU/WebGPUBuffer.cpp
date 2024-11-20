#include "Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
#include "Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
#include <thread>

void WebGPUBuffer::Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) {
    // In webgpu we always create both host and local buffers, unlike vulkan were we can map staging buffers
//    if(!(type & EBufferType::BT_LOCAL)) {
//        type = (EBufferType)(type | EBufferType::BT_LOCAL);
//    }
    
//
    
    // In WebGPU if we are creating a uniform buffer as only host type, unlike vulkan we need to create both buffers (staging and local)
    if(type == EBufferType::BT_HOST && EBufferUsage::BU_Uniform) {
        type = (EBufferType)(type | EBufferType::BT_LOCAL);
    }
    
    Buffer::Initialize(type, usage, allocSize);

    WebGPUDevice* device = (WebGPUDevice*)_device;
    if(!device) {
        assert(false && "Invalid device during buffer initialization");
        return;
    }
    
    WGPUDevice wgpuDevice = device->GetWebGPUDevice();
    if(!wgpuDevice) {
        assert(false && "Invalid web gpu device during buffer intialization");
        return;
    }
    
    if(_type & EBufferType::BT_HOST) {
        WGPUBufferUsage hostBufferUsage = WGPUBufferUsage::WGPUBufferUsage_MapWrite;
        hostBufferUsage = (WGPUBufferUsage)(hostBufferUsage | WGPUBufferUsage::WGPUBufferUsage_CopySrc);
                
        WGPUBufferDescriptor bufferDescriptor {};
        bufferDescriptor.label = "Host Buffer";
        bufferDescriptor.size = allocSize;
        bufferDescriptor.usage = hostBufferUsage;
        bufferDescriptor.mappedAtCreation = true;
        bufferDescriptor.nextInChain = nullptr;

        _bIsMapped = true;
        
        _staginBuffer = wgpuDeviceCreateBuffer(wgpuDevice, &bufferDescriptor);
        if(!_staginBuffer) {
            assert(false && "Unable to create host buffer");
            return;
        }
    }
    
    WGPUBufferUsage bufferUsage = WGPUBufferUsage_None;
    if(usage & EBufferUsage::BU_Geometry) {
        bufferUsage = (WGPUBufferUsage)(WGPUBufferUsage::WGPUBufferUsage_Vertex | WGPUBufferUsage::WGPUBufferUsage_Index);
    }
    
    if(usage & EBufferUsage::BU_Uniform) {
        bufferUsage = WGPUBufferUsage::WGPUBufferUsage_Uniform;
    }
    
    if(_type & EBufferType::BT_LOCAL) {

        bufferUsage = (WGPUBufferUsage)(bufferUsage | WGPUBufferUsage::WGPUBufferUsage_CopyDst);
        
        if(bufferUsage == WGPUBufferUsage::WGPUBufferUsage_None) {
            assert(false);
            return;
        }
        
        WGPUBufferDescriptor bufferDescriptor {};
        bufferDescriptor.label = "Local Buffer";
        bufferDescriptor.size = allocSize;
        bufferDescriptor.usage = bufferUsage;
        bufferDescriptor.mappedAtCreation = false;
        bufferDescriptor.nextInChain = nullptr;
        
        _localBuffer = wgpuDeviceCreateBuffer(wgpuDevice, &bufferDescriptor);
        if(!_localBuffer) {
            assert(false && "Unable to create local buffer");
            return;
        }
    }
}

void *WebGPUBuffer::LockBuffer() {
//    // Define a callback for when the map operation completes
//    auto mapCallback = [](WGPUBufferMapAsyncStatus status, void* userData) {
//        auto* buffer = static_cast<WebGPUBuffer*>(userData);
//        std::lock_guard<std::mutex> lock(buffer->mappingMutex);
//
//        if (status == WGPUBufferMapAsyncStatus_Success) {
//            // Successfully mapped, get the mapped memory
//            buffer->_mappedMemory = wgpuBufferGetMappedRange(buffer->_staginBuffer, 0, buffer->_size);
//        } else {
//            buffer->_mappedMemory = nullptr;
//        }
//        
//        buffer->_bMappedCompleted = true;
//        buffer->_mappedMemoryCondition.notify_one();
//    };
    
    WebGPUDevice* device = (WebGPUDevice*)_device;
    if(!device) {
        assert(false && "Invalid device during buffer initialization");
        return nullptr;
    }
    
    WGPUDevice wgpuDevice = device->GetWebGPUDevice();
    if(!wgpuDevice) {
        assert(false && "Invalid web gpu device during buffer intialization");
        return nullptr;
    }

    
    WGPUBufferMapCallback mapCallback = [](WGPUBufferMapAsyncStatus status, void * userdata) {
        auto* buffer = static_cast<WebGPUBuffer*>(userdata);
        if(buffer) {
            buffer->_bIsMapped = status == WGPUBufferMapAsyncStatus_Success;
        }
    };
    
    if(!_bIsMapped) {
        wgpuBufferMapAsync(_staginBuffer, WGPUMapMode::WGPUMapMode_Write, 0, _size, mapCallback, this);
    }
    
    while(!_bIsMapped) {
        // todo create utility function to pool device when needed
#if defined(WEBGPU_BACKEND) && !defined(__EMSCRIPTEN__)
            wgpuDevicePoll(wgpuDevice, true, nullptr);
#endif

#if defined(__EMSCRIPTEN__)
    emscripten_sleep(100);
#endif
    }
    
    _mappedMemory = wgpuBufferGetMappedRange(_staginBuffer, 0, _size);
    
//    struct mapAsyncRequestData {
//        WGPUAdapter adapter = nullptr;
//        bool requestEnded = false;
//    };
//
//    
//
//    // Start the async mapping operation
//    wgpuBufferMapAsync(_staginBuffer, WGPUMapMode_Write, 0, _size, mapCallback, this);
//
//    std::unique_lock<std::mutex> lock(mappingMutex);
//    _mappedMemoryCondition.wait(lock, [this] { return _bMappedCompleted; });
    
    return _mappedMemory;
}

void WebGPUBuffer::UnlockBuffer() {
    if (_mappedMemory) {
        wgpuBufferUnmap(_staginBuffer);
        _mappedMemory = nullptr;

        // Reset the mapCompleted flag for future LockBuffer calls
        _bIsMapped = false;
    }
}
