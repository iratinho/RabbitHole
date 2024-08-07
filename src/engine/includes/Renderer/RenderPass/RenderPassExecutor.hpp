#pragma once
#include "RenderPassGenerator.hpp"

class MeshComponent;
class CommandPool;
class FrameResources;

class RenderPassExecutor {
public:
    RenderPassExecutor() = default;
    RenderPassExecutor(RenderContext* renderContext, CommandPool* commandPool, RenderPassGenerator&& generator, const std::string& passIdentifier, FrameResources* frameResources);

    bool Execute(unsigned int frameIndex);

private:
    RenderContext* _renderContext;
    CommandPool* _commandPool;
    RenderPassGenerator _generator;
    MeshComponent* _sceneComponent;
    FrameResources* _frameResources;
    std::string _passIdentifier;
};
