#pragma once
#include "Renderer/RenderPass/RenderPass.hpp"
#include "Renderer/Buffer.hpp"

class Buffer;
struct MeshNode;
class CommandPool;

struct BufferActionData {
    Buffer* _buffer;
};

struct BufferAllocateAction : public BufferActionData {
    EBufferType _bufferType;
    size_t _allocationSize;
};

struct BufferStageGeometryDataActionData : public BufferActionData {
    const MeshNode* _meshNode;
    RenderContext* _renderContext;
};

struct BufferUploadActionData : public BufferActionData {
    CommandPool* _commandPool;
};

class BufferAction : public IGraphAction {
public:
    BufferAction() = delete;
    BufferAction(const std::any& actionData);
    bool Execute() override;

private:
    bool StageGeometryData(const BufferStageGeometryDataActionData& data);
};
