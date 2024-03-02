#pragma once

class CommandQueue;

class CommandBuffer {
public:    
    virtual void Submit() = 0;
    
private:
    CommandQueue* _commandQueue = nullptr;
};
