#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"
#include "Renderer/Buffer.hpp"

class Buffer;
struct MeshNode;
class CommandBuffer;

struct BufferActionData {
    std::shared_ptr<Buffer> _buffer;
};

struct BufferAllocateActionData : public BufferActionData {
    EBufferType _bufferType;
    size_t _allocationSize;
};

struct BufferStageGeometryDataActionData : public BufferActionData {
    const MeshNode* _meshNode;
    RenderContext* _renderContext;
};

struct BufferUploadActionData : public BufferActionData {
    CommandBuffer* _commandBuffer;
};

class BufferAction : public IGraphAction {
public:
    BufferAction() = delete;
    BufferAction(const std::any& actionData);
    bool Execute() override;

private:
    bool StageGeometryData(const BufferStageGeometryDataActionData& data);
};
