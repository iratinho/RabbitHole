#pragma once
#include "Core/Components/MeshComponent.hpp"
#include "Renderer/render_context.hpp"
#include "entt/entt.hpp"
#include "RenderGraph/Actions/SurfaceAction.hpp"
#include "Core/Utils.hpp"

class Surface;
class CommandPool;
class RenderTarget;
class RenderGraph;
class RenderContext;
class IRenderer;
class Fence;
class GraphBuilder;
class RenderPassGenerator;
class Scene;
class Mesh;

struct PresistentRenderTargets {
    RenderTarget* scene_color_render_target;
    RenderTarget* scene_depth_render_target;
};

struct SyncPrimitives {
    VkFence in_flight_fence;
    VkSemaphore render_finish_semaphore;
    std::shared_ptr<Fence> in_flight_fence_new;
};

struct FrameData {
    SyncPrimitives sync_primitives;
    std::shared_ptr<CommandPool> _commandPool;
    std::shared_ptr<Surface> _presentableSurface;
};
    
class RenderSystem {
public:
    bool Initialize(InitializationParams initialization_params);
    bool Process(Scene* scene);
    void HandleResize(int width, int height);

    uint32_t GetCurrentFrameIndex() { return _frameIndex; };

    RenderContext* GetRenderContext() const { return _renderContext; }
    RenderGraph* GetRenderGraph() const { return _renderGraph; }

    // todo IMRPOVE THIS
    bool ReleaseResources();
    
    template <typename T>
    static std::vector<char> MakePaddedGPUBuffer(const T& data) {
        std::vector<char> ret;
        auto writeData = [&ret](const auto& m){
            const size_t mSize = sizeof(m);
            size_t padding = (mSize % 8 != 0 && mSize % 8 != mSize) ? (mSize + (8 - mSize % 8)) - mSize : 0;
            
            if(mSize == 4) {
                padding = 12;
            }

            const char* mBytes = reinterpret_cast<const char*>(&m);
            for (size_t i = 0; i < mSize + padding; ++i) {
                if(i < mSize) {
                    ret.push_back(mBytes[i]);
                } else {
                    ret.push_back('\0');
                }
            }
        };

        if constexpr (std::is_aggregate_v<T>) {
            for_each_member(data, writeData);
        } else {
            const char* bytes = reinterpret_cast<const char*>(&data);
            const size_t size = sizeof(T);
            const T* data = reinterpret_cast<const T*>(bytes);
            size_t padding = (size % 8 != 0) ? (8 - size % 8) : 0;
            
            if(size == 4) {
                padding = 12;
            }

            ret = std::vector<char>(bytes, bytes + size);
            
            data = reinterpret_cast<const T*>(ret.data());
            for (size_t i = 0; i < padding; ++i) {
                    ret.push_back('\0');
            }
            data = reinterpret_cast<const T*>(ret.data());
        }
        
        return ret;
    }
    
    template <typename T>
    size_t CalculateGPUDStructSize() {
        T dummy {};
        
        size_t size = 0;
        
        auto writeData = [&size](const auto& m){
            const size_t mSize = sizeof(m);
            size_t padding = (mSize % 8 != 0 && mSize % 8 != mSize) ? (mSize + (8 - mSize % 8)) - mSize : 0;
            
            if(mSize == 4) {
                padding = 12;
            }
            
            size += mSize + padding;
        };

        if constexpr (std::is_aggregate_v<T>) {
            for_each_member(dummy, writeData);
        } else {
            const size_t mSize = sizeof(T);
            size_t padding = (mSize % 8 != 0) ? (8 - mSize % 8) : 0;

            if(mSize == 4) {
                padding = 12;
            }

            size += mSize + padding;
        }

        return size;
    }
    
private:
    void AllocateGeometryBuffers(GraphBuilder* graphBuilder, unsigned frameIndex);
    bool CreateSyncPrimitives();
    void GenerateSceneProxies(const MeshComponent* sceneComponent, RenderPassGenerator* renderPassGenerator);
    void GenerateSceneProxies(RenderPassGenerator* renderPassGenerator, std::function<void(const MeshNode*, const PrimitiveData*)> func = [](const MeshNode*, const PrimitiveData*){}, std::function<bool(const Mesh*)> filter = [](const Mesh*){ return true; });
    void SetupOpaqueRenderPass(GraphBuilder* graphBuilder);
    void SetupFloorGridRenderPass(GraphBuilder* graphBuilder);

    InitializationParams m_InitializationParams;

    RenderContext* _renderContext;
    RenderGraph* _renderGraph;
    Scene* _activeScene;

    std::vector<FrameData> _frameData;
    uint32_t _frameIndex = 0;

    bool needs_swapchain_recreation = false;
    bool invalid_surface_for_swapchain = false;
};
