#pragma once
#include "glm/glm.hpp"

class GraphicsPipeline;

struct RenderPassValues {
    glm::vec3 clearColor;
    glm::vec2 clearDepth;
    
    unsigned int viewportX;
    unsigned int viewportY;
    unsigned int viewportWidth;
    unsigned int viewportHeight;
    
    float minZ = 0.0;
    float maxZ = 1.0;
    
    unsigned int scissorX;
    unsigned int scissorY;
    unsigned int scissorWidth;
    unsigned int scissorHeight;
};

class RenderPass {
public:    
    virtual ~RenderPass() = default;
    
    /**
     * @brief Creates a render pass backed by a pipeline
     * @param pipeline used to create a compatible render pass
     */
    static std::shared_ptr<RenderPass> Create(std::shared_ptr<GraphicsPipeline> graphicsPipeline);
    
    /**
     * @brief Initiates render pass operations
     */
    virtual void BeginRenderPass() = 0;
    
    /**
     * @brief Ends render pass operations
     */
    virtual void EndRenderPass() = 0;
        
    /**
     * @brief Sets the initial clear color for render pass render targets
     * @param color - The color used to clear the render target
     */
    void SetClearColor(const glm::vec3& color);
    
    /**
     * @brief Sets the initial clear depth value for render pass depth/stencil render targets
     *
     * @param value - The depth value  used to clear the depth/stencil render target
     */
    void SetClearDepth(const glm::vec2& value);
    
    /**
     * @brief Specifies the 3D rectangular region for the viewport clipping
     * @param x - The x coordinate of the upper-left corner of the viewport.
     * @param y - The y coordinate of the upper-left corner of the viewport.
     * @param width - The width of the viewport, in pixels.
     * @param height - The height of the viewport, in pixels.
     */
    void SetViewportRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    
    /**
     * @brief Specifies the Z coordinate of the near clipping plane
     * @param minZ - The minZ value (minDepth)
     */
    void SetMinZ(float minZ);
    
    /**
     * @brief Specifies the Z coordinate of the near clipping plane
     * @param minZ - The maxZ value (maxDepth)
     */
    void SetMaxZ(float maxZ);
    
    /**
     * @brief Specifies the rectangle for the scissor fragment test
     * @param x - The x window coordinate of the upper-left corner of the scissor rectangle.
     * @param y - The y window coordinate of the upper-left corner of the scissor rectangle.
     * @param width - The width of the scissor rectangle, in pixels.
     * @param height - The height of the scissor rectangle, in pixels.
     */
    void SetScissorRect(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
    
    void SetDepthStencil() {};
    
    void SetTriangleCullMode() {};
    
    void SetTriangleWinding() {};
    
    /**
     * @brief Specifies  a render pass texture read dependency
     * @param identifier - The texture identifier to identify the texture
     */
    void SetTextureInput(const std::string& identifier) {};
    
    /**
     * @brief Specifies render pass texture write dependency
     * @param identifier - The texture identifier to identify the texture
     */
    void SetTextureOutput(const std::string& identifier) {};
    
protected:
    RenderPassValues _renderPassValues = {};
};
