#include "Renderer/RenderGraph/Actions/BufferAction.h"

#include "Core/Components/SceneComponent.h"
#include "Renderer/Buffer.h"

bool BufferAction::Execute() {
    if(_renderContext == nullptr && _buffer == nullptr) {
        return false;
    }
    
    switch (_bufferAction) {
        case BA_Empty: return false;
        case BA_AllocateCPUBuffer: return _buffer->AllocateBuffer(_allocationSize, EBufferUsage::BU_Geometry, true);
        case BA_AllocateGPUBuffer: return _buffer->AllocateBuffer(_allocationSize, EBufferUsage::BU_Geometry, false);
        case BA_StageGeometryData: return StageGeometryData();
        case BA_TransferToGPU: return _buffer->Upload(_commandBuffer);
    }
    
    return false;
}

bool BufferAction::StageGeometryData() {
    if(!_buffer && !_meshNode) {
        return false;
    }
    
    // Temporary buffer to have the data in a vulkan cpu backed buffer
    _buffer->_stagingBuffer = std::make_unique<Buffer>(_renderContext);

    // Compute the necessary allocation size for our buffers
    size_t allocationSize = 0;
    for (const PrimitiveData& primitiveData : _meshNode->_primitives) {
        allocationSize += primitiveData._indices.size() * sizeof(unsigned) + primitiveData._vertexData.size() * sizeof(VertexData);
    }

    if(allocationSize <= 0) {
        std::cerr << "[Error]: Trying to allocate a staging buffer with an invalid allocation size." << std::endl;
        return false;
    }
    
    // Staging Buffer
    _buffer->_stagingBuffer->AllocateBuffer(allocationSize, EBufferUsage::BU_Geometry, true);
    unsigned char* bufferAlloc = (unsigned char*)_buffer->_stagingBuffer->Lock();

    // Copy data to staging buffer
    for (const PrimitiveData& primitiveData : _meshNode->_primitives) {
        // Copy Indices
        memcpy(bufferAlloc, primitiveData._indices.data(), primitiveData._indices.size() * sizeof(unsigned));
        // Copy VertexData
        memcpy(bufferAlloc + primitiveData._indices.size() * sizeof(unsigned), primitiveData._vertexData.data(), primitiveData._vertexData.size() * sizeof(VertexData));
    }

    _buffer->_stagingBuffer->Unlock();
    _buffer->_allocationSize = allocationSize;

    return true;
}
