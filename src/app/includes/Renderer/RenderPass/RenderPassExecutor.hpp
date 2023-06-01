#pragma once
#include "RenderPassGenerator.hpp"

class MeshComponent;
class CommandPool;

class RenderPassExecutor {
public:
    RenderPassExecutor() = default;
    RenderPassExecutor(RenderContext* renderContext, CommandPool* commandPool, RenderPassGenerator&& generator);

    bool Execute(unsigned int frameIndex);

private:
    RenderContext* _renderContext;
    CommandPool* _commandPool;
    RenderPassGenerator _generator;
    MeshComponent* _sceneComponent;
};