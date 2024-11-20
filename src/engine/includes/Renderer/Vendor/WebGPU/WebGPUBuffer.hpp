#pragma once
#include "Renderer/Buffer.hpp"
#include "webgpu/webgpu.hpp"
#include <condition_variable>
#include <mutex>

class WebGPUBuffer : public Buffer {
public:
    void Initialize(EBufferType type, EBufferUsage usage, size_t allocSize) override;
    void* LockBuffer() override;
    void UnlockBuffer() override;
    
    WGPUBuffer GetHostBuffer() const {
        return _staginBuffer;
    }
    
    WGPUBuffer GetLocalBuffer() const {
        return _localBuffer;
    }
    
private:
    WGPUBuffer _staginBuffer = nullptr;
    WGPUBuffer _localBuffer = nullptr;

    void* _mappedMemory = nullptr;
    std::condition_variable _mappedMemoryCondition;
    std::mutex mappingMutex;
    bool _bIsMapped;
};
