#pragma once
#include "Renderer/render_context.hpp"

class Buffer;

struct PrimitiveData {
    std::string _primitiveName {};
    std::vector<unsigned int> _indices {};
    std::vector<VertexData> _vertexData {};
    unsigned _indicesOffset = 0;
    unsigned int _vertexOffset = 0;
    unsigned int _dataOffset = 0;
};

struct MeshNode {
    std::string _nodeName;
    std::string _filePath; // Specifies the path from where this geometry was loaded from
    std::vector<PrimitiveData> _primitives {}; // Primitives for the current mesh
    std::vector<MeshNode> _childNode {};
    std::shared_ptr<Buffer> _buffer; // gpu buffer that has primitive data for the entire mesh
    glm::mat4x4 _transformMatrix = glm::mat4(1.0f); // Original matrix from the imported mesh
    glm::mat4x4 _computedMatrix = glm::mat4(1.0f); // Used when calculating child matrix for a mesh
    bool _bWasProcessed = false;
};

struct MeshComponent {
    std::vector<MeshNode>  _meshNodes;
    
    bool _renderMainPass = true; // by default we always render to main pass
    bool _renderFloorGridPass = false;
};
