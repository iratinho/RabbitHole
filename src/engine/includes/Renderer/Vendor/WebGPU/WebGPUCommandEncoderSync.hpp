#pragma once

class WebGPURenderCommandEncoder;
class WebGPUBlitCommandEncoder;

class WebGPUCommandEncoderSync
{    
public:
    enum SyncCommandEncoderTag {
        RenderCommandEncoder,
        BlitCommandEncoder
    };
    
    struct SyncCommandEncoderData
    {
        void* _commandEncoder;
        SyncCommandEncoderTag _tag;
    };

    template <typename T>
    static constexpr SyncCommandEncoderTag GetTag() {
        if constexpr (std::is_same<T, WebGPURenderCommandEncoder>::value) {
            return RenderCommandEncoder;
        } else if constexpr (std::is_same<T, WebGPUBlitCommandEncoder>::value) {
            return BlitCommandEncoder;
        }
        
        return {};
    }

    
public:
    // Create singleton class
    static WebGPUCommandEncoderSync& GetInstance()
    {
        static WebGPUCommandEncoderSync instance;
        return instance;
    }

    template <typename T>
    void SyncCommandEncoder(const T* commandEncoder)
    {
        SyncCommandEncoderData data = {(void*)commandEncoder, GetTag<T>()};
        _syncData.push(data);
    }

    SyncCommandEncoderData GetNextCommandEncoder()
    {
        if (_syncData.empty())
            return {};

        auto data = _syncData.front();
        _syncData.pop();
        
        return data;
    }

    std::queue<SyncCommandEncoderData> _syncData;
};
