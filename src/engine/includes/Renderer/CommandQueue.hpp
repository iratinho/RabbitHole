#pragma once

class CommandBuffer;

class CommandQueue {
public:
    virtual ~CommandQueue() = default;
    
    /*
     * @brief Returns a command buffer allocated with this command queue
     */
    virtual CommandBuffer* MakeCommandBuffer() = 0;
    
private:
    std::vector<std::unique_ptr<CommandBuffer>> _commandBufferPool;
};
