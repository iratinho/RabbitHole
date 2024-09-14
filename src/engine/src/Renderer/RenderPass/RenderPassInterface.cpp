#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphicsPipeline.hpp"

namespace {
    
    void UpdatePushConstantsOffsets(std::vector<PushConstant>& pushConstants) {
        std::size_t offset = 0;
        for(PushConstant& constant : pushConstants) {
            constant._offset = offset;
            offset += constant._size;
        }
    }
    
    std::pair<ShaderParams, ShaderParams> MakeShaderParams(RenderPass* renderPass) {
        if(!renderPass) {
            return {};
        }
        
        auto pushConstants = renderPass->CollectPushConstants();
        UpdatePushConstantsOffsets(pushConstants);
        
        const auto& resourceBindings = renderPass->CollectResourceBindings();

        ShaderParams vsParams;
        vsParams._shaderPath = renderPass->GetVertexShaderPath();
        vsParams._shaderInputBindings = renderPass->CollectShaderInputBindings();
        
        // Copy push constants data only for vertex shader
        std::copy_if(pushConstants.begin(), pushConstants.end(), std::back_inserter(vsParams._shaderInputConstants), [](const PushConstant& x) {
            return x._shaderStage == ShaderStage::STAGE_VERTEX;
        });
        
        // Copy shade resource bindings data only for vertex shader
        std::copy_if(resourceBindings.begin(), resourceBindings.end(), std::back_inserter(vsParams._shaderResourceBindings), [](const ShaderResourceBinding& x) {
            return x._shaderStage == ShaderStage::STAGE_VERTEX;
        });

        ShaderParams fsParams;
        fsParams._shaderPath = renderPass->GetFragmentShaderPath();
        
        // Copy push constants data only for vertex shader
        std::copy_if(pushConstants.begin(), pushConstants.end(), std::back_inserter(fsParams._shaderInputConstants), [](const PushConstant& x) {
            return x._shaderStage == ShaderStage::STAGE_FRAGMENT;
        });
        
        // Copy shade resource bindings data only for vertex shader
        std::copy_if(resourceBindings.begin(), resourceBindings.end(), std::back_inserter(fsParams._shaderResourceBindings), [](const ShaderResourceBinding& x) {
            return x._shaderStage == ShaderStage::STAGE_FRAGMENT;
        });

        return {vsParams, fsParams};
    }
}


void RenderPass::Initialize(GraphicsContext* graphicsContext) {
    GraphicsPipelineParams params = GetPipelineParams();
    params._renderAttachments = GetRenderAttachments(graphicsContext);
    params._device = graphicsContext->GetDevice();
    
    std::tie(params._vsParams, params._fsParams) = MakeShaderParams(this);
    
    _pipeline = GraphicsPipeline::Create(params);
    _pipeline->Compile();    
}
