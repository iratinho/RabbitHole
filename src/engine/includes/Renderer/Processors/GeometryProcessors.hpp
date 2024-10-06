#pragma once
#include "Components/PrimitiveProxyComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Renderer/Processors/MaterialProcessors.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/Buffer.hpp"
#include "Core/Scene.hpp"

class Device;
class GraphicsPipeline;
class GraphicsContext;

namespace {
    // Function to print vertex data
    void printVertexData(unsigned char* data, size_t vertexCount) {
        // Cast the unsigned char pointer to a VertexData pointer
        VertexData* vertices = reinterpret_cast<VertexData*>(data);

        for (size_t i = 0; i < vertexCount; ++i) {
            VertexData& vertex = vertices[i]; // Reference for easier access

            std::cout << "Vertex " << i << ":\n";
            std::cout << "  Position: ("
                      << vertex.position.x << ", "
                      << vertex.position.y << ", "
                      << vertex.position.z << ")\n";
            std::cout << "  Normal: ("
                      << vertex.normal.x << ", "
                      << vertex.normal.y << ", "
                      << vertex.normal.z << ")\n";
            std::cout << "  TexCoords: ("
                      << vertex.texCoords.x << ", "
                      << vertex.texCoords.y << ")\n";
            std::cout << "  Color: ("
                      << vertex.color.x << ", "
                      << vertex.color.y << ", "
                      << vertex.color.z << ")\n";
            std::cout << std::endl; // New line for better readability
        }
    }
}

template <typename Child>
class GeometryProcessor {
public:
    // Generates a new scene buffer with all the geometry that needs to be uploaded, returns null if its not necessary
    static std::shared_ptr<Buffer> GenerateBuffer(Device* device, Scene *scene) {
        return Child::GenerateBufferImp(device, scene);
    };
    
    // TODO: This mesh relevance should be used when we are trying to load textures for a mesh and only load if its relevant, the same for drawing
    template <typename MaterialComponent>
    static void CalculateMeshRelevance() {};

    template <typename MaterialComponent>
    static void Draw(Device* device, GraphicsContext* graphicsContext, Scene* scene, RenderCommandEncoder* encoder, GraphicsPipeline* pipeline) {
        Child::template DrawImp<MaterialComponent>(device, graphicsContext, scene, encoder, pipeline);
    };
};

class MeshProcessor : public GeometryProcessor<MeshProcessor> {
public:
    static std::shared_ptr<Buffer> GenerateBufferImp(Device* device, Scene *scene) {
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

        auto buffer = Buffer::Create(device);
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

//            printVertexData(bufferPtr + bufferOffset + indicesSize, proxyComponent._vertexData.size());

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
