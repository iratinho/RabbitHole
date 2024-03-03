#pragma once
#include "Components/Common.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/GPUDefinitions.h"

class PrimitiveProxyComponent : public CommonComponent
{
public:
    DECLARE_CONSTRUCTOR(PrimitiveProxyComponent, CommonComponent)

    /*
     *  This buffer can be the same in multiple primitives components from the same mesh
     * we then use the offsets to gather the relevant portion of data from this big buffer
     */
    std::shared_ptr<Buffer> _gpuBuffer;
    
    // Computed per frame
    glm::mat4x4 _transformMatrix;
    unsigned _indicesOffset = 0;
    unsigned int _vertexOffset = 0;
    unsigned int _indicesCount = 0;
};

// We separate the CPU data into a specific component, so that when rendering we dont need to use much cache space
// Since most of the time this data is not needed to render the mesh. It will already be in the GPU.
class PrimitiveProxyComponentCPU : public CommonComponent
{
public:
    DECLARE_CONSTRUCTOR(PrimitiveProxyComponentCPU, CommonComponent)
    std::vector<unsigned int> _indices{};
    std::vector<VertexData> _vertexData{};
};
