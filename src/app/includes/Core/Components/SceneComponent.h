#pragma once
#include "Renderer/render_context.h"

class Buffer;

struct PrimitiveData {
    std::string _primitiveName {};
    std::vector<unsigned int> _indices {};
    std::vector<VertexData> _vertexData {};
    unsigned int _dataOffset = 0;
};

struct MeshNode {
    std::string _nodeName;
    std::string _filePath; // Specifies the path from where this geometry was loaded from
    std::vector<PrimitiveData> _primitives {}; // Primitives for the current mesh
    std::vector<MeshNode> _childNode {};
    std::shared_ptr<Buffer> _buffer; // gpu buffer that has primitive data for the entire mesh
    bool _bWasProcessed = false;
};

struct SceneComponent {
    std::vector<MeshNode>  _meshNodes;
};