#pragma once
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/RenderCommandEncoder.hpp"
#include "Renderer/Buffer.hpp"
#include "Core/Scene.hpp"

class RenderContext;
class GraphicsPipeline;
class GraphicsContext;

template <typename Child>
class GeometryProcessor {
public:
    // Generates a new scene buffer with all the geometry that needs to be uploaded, returns null if its not necessary
    static std::shared_ptr<Buffer> GenerateBuffer(RenderContext* renderContext, Scene *scene) {
        return Child::GenerateBufferImp(renderContext, scene);
    };

    template <typename MaterialComponent>
    static void Draw(RenderContext* renderContext, GraphicsContext* graphicsContext, Scene* scene, RenderCommandEncoder* encoder, GraphicsPipeline* pipeline) {
        Child::template DrawImp<MaterialComponent>(renderContext, graphicsContext, scene, encoder, pipeline);
    };
};

class MeshProcessor : public GeometryProcessor<MeshProcessor> {
public:
    static std::shared_ptr<Buffer> GenerateBufferImp(RenderContext* renderContext, Scene *scene) {
        auto view = scene->GetRegistry().view<PrimitiveProxyComponentCPU>();

        // Calculate how much space we need to allocate for primitives
        size_t allocationSize = 0;
        for (auto entity : view) {
            const PrimitiveProxyComponentCPU& proxyComponent = scene->GetRegistry().get<PrimitiveProxyComponentCPU>(entity);
        
            // Skip in case we already have a PrimitiveProxyComponent that signals that we have gpu data
            if(scene->GetRegistry().any_of<PrimitiveProxyComponent>(entity)) {
                continue;
            }
        
            allocationSize += proxyComponent._indices.size() * sizeof(unsigned int) + proxyComponent._vertexData.size() * sizeof(VertexData);
        }
    
        // There is no need to process any geometry, since there is nothing to allocate
        if(allocationSize == 0) {
            return nullptr;
        }

        auto buffer = Buffer::Create(renderContext);
        buffer->Initialize((EBufferType)(EBufferType::BT_HOST | EBufferType::BT_LOCAL), (EBufferUsage)(EBufferUsage::BU_Geometry | EBufferUsage::BU_Transfer), allocationSize);

        void* bufferAlloc = (unsigned char*)buffer->LockBuffer();
        unsigned char* bufferPtr = static_cast<unsigned char*>(bufferAlloc);

        size_t bufferOffset = 0;
        for (auto entity : view) {
            // Skip in case we already have a PrimitiveProxyComponent that signals that we have gpu data
            if(scene->GetRegistry().any_of<PrimitiveProxyComponent>(entity)) {
                continue;
            }
        
            const PrimitiveProxyComponentCPU& proxyComponent = scene->GetRegistry().get<PrimitiveProxyComponentCPU>(entity);
        
            // Calculate the total size needed for indices and vertex data
            size_t indicesSize = proxyComponent._indices.size() * sizeof(unsigned int);
            size_t vertexDataSize = proxyComponent._vertexData.size() * sizeof(VertexData);
        
            // Copy indices data
            memcpy(bufferPtr + bufferOffset, proxyComponent._indices.data(), indicesSize);
        
            // Copy vertex data after the indices data
            memcpy(bufferPtr + bufferOffset + indicesSize, proxyComponent._vertexData.data(), vertexDataSize);

            PrimitiveProxyComponent gpuProxyComponent;
            gpuProxyComponent._gpuBuffer = buffer;
            gpuProxyComponent._indicesCount = proxyComponent._indices.size();
            gpuProxyComponent._indicesOffset = bufferOffset;
            gpuProxyComponent._vertexOffset = bufferOffset + indicesSize;
        
            scene->GetRegistry().emplace<PrimitiveProxyComponent>(entity, gpuProxyComponent);
        
            bufferOffset += indicesSize + vertexDataSize;
        }

        buffer->UnlockBuffer();

        return buffer;
    };

    template <typename MaterialComponent>
    static void DrawImp(RenderContext* renderContext, GraphicsContext* graphicsContext, Scene* scene, RenderCommandEncoder* encoder, GraphicsPipeline* pipeline) {
        
        // TODO Upload geometry here in this function if necessary
        
        using Components = std::tuple<PrimitiveProxyComponent, MaterialComponent>;
        const auto& view = scene->GetRegistryView<Components>();
        for(auto entity : view) {
            MaterialProcessor<MaterialComponent>::template Process<decltype(entity)>(graphicsContext, encoder, pipeline, scene, entity);

            const auto& proxy= view.template get<PrimitiveProxyComponent>(entity);
            encoder->DrawPrimitiveIndexed(proxy);
        }
    };
};

class PointCloudProcessor : public GeometryProcessor<PointCloudProcessor> {
public:
    template <typename MaterialComponent>
    static void DrawImp(RenderContext* device, Scene* scene, RenderCommandEncoder* encoder, GraphicsPipeline* pipeline) {
        // not implemented yet
        assert(false);
    };
};
