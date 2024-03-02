#include "Renderer/RenderPass.hpp"


std::shared_ptr<RenderPass> RenderPass::Create(std::shared_ptr<GraphicsPipeline> graphicsPipeline) { 
    return nullptr;
}

void RenderPass::SetClearColor(const glm::vec3 &color) { 
    _renderPassValues.clearColor = color;
}

void RenderPass::SetClearDepth(const glm::vec2 &value) {
    _renderPassValues.clearDepth = value;
}

void RenderPass::SetViewportRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) { 
    _renderPassValues.viewportX = x;
    _renderPassValues.viewportY = y;
    _renderPassValues.viewportWidth = width;
    _renderPassValues.viewportHeight = height;
}

void RenderPass::SetMinZ(float minZ) { 
    _renderPassValues.minZ = minZ;
}

void RenderPass::SetMaxZ(float maxZ) { 
    _renderPassValues.maxZ = maxZ;
}

void RenderPass::SetScissorRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height) { 
    _renderPassValues.scissorX = x;
    _renderPassValues.scissorY = y;
    _renderPassValues.scissorWidth = width;
    _renderPassValues.scissorHeight = height;
}
