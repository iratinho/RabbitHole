#include "Renderer/RenderGraph/Actions/BufferAction.hpp"
#include "Core/Components/SceneComponent.hpp"
#include "Renderer/Buffer.hpp"

BufferAction::BufferAction(const std::any& actionData) {
    IGraphAction::_actionData = actionData;
}

bool BufferAction::Execute() {
    // Allocate
    if(BufferAllocateActionData* data = std::any_cast<BufferAllocateActionData>(&_actionData)) {
        if(!data->_buffer && data->_allocationSize != 0){
            data->_buffer->AllocateBuffer(data->_allocationSize,
                                          EBufferUsage::BU_Geometry,
                                          data->_bufferType == EBufferType::GPU ? false : true);
            return true;
        }
    }
    
    // Stage geometry
    if(BufferStageGeometryDataActionData* data = std::any_cast<BufferStageGeometryDataActionData>(&_actionData)) {
        if(StageGeometryData(*data)) {
            return true;
        };
    }
    
    // Upload to gpu
    if(BufferUploadActionData* data = std::any_cast<BufferUploadActionData>(&_actionData)) {
        if(!data->_buffer && !data->_commandBuffer) {
            data->_buffer->Upload(data->_commandBuffer);
        }
    }
    
    return false;
}

bool BufferAction::StageGeometryData(const BufferStageGeometryDataActionData& data) {
    if(data._buffer && data._meshNode) {
        // Temporary buffer to have the data in a vulkan cpu backed
        data._buffer->_stagingBuffer = std::make_unique<Buffer>(data._renderContext);
        
        // Compute the necessary allocation size for our buffers
        size_t allocationSize = 0;
        for (const PrimitiveData& primitiveData : data._meshNode->_primitives) {
            allocationSize += primitiveData._indices.size() * sizeof(unsigned) + primitiveData._vertexData.size() * sizeof(VertexData);
        }
        
        if(allocationSize <= 0) {
            std::cerr << "[Error]: Trying to allocate a staging buffer with an invalid allocation size." << std::endl;
            return false;
        }
        
        // Staging Buffer
        data._buffer->_stagingBuffer->AllocateBuffer(allocationSize, EBufferUsage::BU_Geometry, true);
        unsigned char* bufferAlloc = (unsigned char*)data._buffer->_stagingBuffer->Lock();
        
        // Copy data to staging buffer
        for (const PrimitiveData& primitiveData : data._meshNode->_primitives) {
            // Copy Indices
            memcpy(bufferAlloc,
                   primitiveData._indices.data(),
                   primitiveData._indices.size() * sizeof(unsigned));
            // Copy VertexData
            memcpy(bufferAlloc + primitiveData._indices.size() * sizeof(unsigned),
                   primitiveData._vertexData.data(),
                   primitiveData._vertexData.size() * sizeof(VertexData));
        }
        
        data._buffer->_stagingBuffer->Unlock();
        data._buffer->_allocationSize = allocationSize;
    }
    
    return true;
}
