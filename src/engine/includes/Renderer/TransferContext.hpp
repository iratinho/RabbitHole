#pragma once

class RenderContext;
class Buffer;

class TransferContext {
public:
    virtual ~TransferContext() = default;
    
    static std::unique_ptr<TransferContext> Create(RenderContext* renderContext);
      
    /*
     * @brief Enqueues a copy command to copy the cpu data to the gpu
     */
    virtual void EnqueueBufferSync(std::shared_ptr<Buffer> buffer) = 0;
    
    /*
     * @brief Processes all buffer transfer operations
     */
    virtual void Flush() = 0;
    
protected:
    RenderContext* _renderContext = nullptr;
};
