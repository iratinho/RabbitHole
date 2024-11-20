#pragma once
#include "Renderer/GraphicsPipeline.hpp"

#include "webgpu/webgpu.hpp"

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

class WebGPUGraphicsPipeline final : public GraphicsPipeline {
public:
    explicit WebGPUGraphicsPipeline(const GraphicsPipelineParams& params)
        : GraphicsPipeline(params) {
    }

    void Compile() override;
    
    [[nodiscard]] WGPURenderPipeline GetWebGPUPipeline() const {
        return _pipeline;
    };
    
    [[nodiscard]] const std::vector<WGPUBindGroupLayout>& GetWebGPUGroupLayouts() const {
        return _bindingGroupLayouts;
    }
    
private:
    std::vector<WGPUBindGroupLayout> _bindingGroupLayouts;
    WGPUPipelineLayout _layout;
    WGPURenderPipeline _pipeline;
    
    bool _bWasCompiled = false;
};
