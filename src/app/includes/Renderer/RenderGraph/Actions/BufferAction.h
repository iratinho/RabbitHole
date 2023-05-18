#pragma once
#include "Renderer/RenderPass/RenderPass.h"

class CommandBuffer;
struct MeshNode;

enum EBufferAction
{
    BA_Empty,
    BA_AllocateCPUBuffer,
    BA_AllocateGPUBuffer,

    BA_StageGeometryData,
    BA_TransferToGPU
};

class Buffer;

class BufferAction : public IGraphAction {
public:
    bool Execute() override;

    EBufferAction _bufferAction;
    CommandBuffer* _commandBuffer;
    std::shared_ptr<Buffer> _buffer;
    size_t _allocationSize = 0;
    RenderContext* _renderContext;
    const MeshNode* _meshNode;

private:
    bool StageGeometryData();
};