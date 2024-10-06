#include "Renderer/RenderPass/UIRenderPass.hpp"

#include <window.hpp>

#include "Renderer/Device.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/TextureResource.hpp"
#include "Renderer/CommandEncoders/RenderCommandEncoder.hpp"
#include "Renderer/VulkanLoader.hpp"

void UIRenderPass::Initialize(GraphicsContext *graphicsContext) {
    if(!graphicsContext->GetDevice()) {
        assert(0);
        return;
    }

    RenderPass::Initialize(graphicsContext);

    if(!_texture) {
        const auto& extent = graphicsContext->GetDevice()->GetSwapchainExtent();
        _texture = Texture2D::MakeTexturePass(extent.x, extent.y, Format::FORMAT_R8G8B8A8_SRGB,
                                              static_cast<TextureFlags>(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_DYNAMIC));
        _texture->Initialize(graphicsContext->GetDevice());
    }

    if(!_ultralightRenderer) {
        _ultralightRenderer = std::make_unique<UltralightRenderer>();
        _ultralightRenderer->Initialize(graphicsContext->GetDevice());
    }

    if(Window* window = graphicsContext->GetDevice()->GetWindow()) {
        window->GetWindowResizeDelegate().AddLambda([this, graphicsContext](glm::i32vec2 newSize) {
            auto resizedTexture = Texture2D::MakeTexturePass(newSize.x, newSize.y, Format::FORMAT_R8G8B8A8_SRGB,
                                      static_cast<TextureFlags>(TextureFlags::Tex_SAMPLED_OP | TextureFlags::Tex_DYNAMIC));
            resizedTexture->Initialize(graphicsContext->GetDevice());

            _texture.reset();
            _texture = resizedTexture;
        });
    }

}

std::string UIRenderPass::GetIdentifier() {
    return "UIRenderPass";
}

RenderAttachments UIRenderPass::GetRenderAttachments(GraphicsContext *graphicsContext) {
    ColorAttachmentBlending blending {};
    blending._colorBlending = BlendOperation::BLEND_OP_ADD;
    blending._alphaBlending = BlendOperation::BLEND_OP_ADD;
    blending._colorBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };
    blending._alphaBlendingFactor = { BlendFactor::BLEND_FACTOR_SRC_ALPHA, BlendFactor::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA };

    ColorAttachmentBinding colorAttachmentBinding {};
    colorAttachmentBinding._texture = graphicsContext->GetSwapChainColorTexture();
    colorAttachmentBinding._blending = blending;
    colorAttachmentBinding._loadAction = LoadOp::OP_LOAD;

    RenderAttachments renderAttachments {};
    renderAttachments._colorAttachmentBinding = colorAttachmentBinding;

    return renderAttachments;
}

GraphicsPipelineParams UIRenderPass::GetPipelineParams() {
    GraphicsPipelineParams pipelineParams;
    pipelineParams._rasterization._triangleCullMode = TriangleCullMode::CULL_MODE_BACK;
    pipelineParams._rasterization._triangleWindingOrder = TriangleWindingOrder::CLOCK_WISE;
    pipelineParams._rasterization._depthCompareOP = CompareOperation::ALWAYS;

    return pipelineParams;
}

ShaderInputBindings UIRenderPass::CollectShaderInputBindings() {
//    ShaderAttributeBinding vertexDataBinding {};
//    vertexDataBinding._binding = 0;
//    vertexDataBinding._stride = sizeof(VertexData);
//
//    // Position vertex input
//    ShaderInputLocation positions {};
//    positions._format = Format::FORMAT_R32G32B32_SFLOAT;
//    positions._offset = offsetof(VertexData, position);
//
//    ShaderInputBindings inputBindings;
//    inputBindings[vertexDataBinding] = {positions};
//    return inputBindings;
    
    return  {};
}

std::vector<PushConstant> UIRenderPass::CollectPushConstants() {
    std::vector<PushConstant> pushConstants;

    PushConstant pushConstant;
    pushConstant.name = "coords";
    pushConstant._dataType = PushConstantDataInfo<float>::_dataType;
    pushConstant._size = PushConstantDataInfo<float>::_gpuSize;
    pushConstant._shaderStage = ShaderStage::STAGE_VERTEX;
    pushConstants.push_back(pushConstant);

    return pushConstants;
}

std::vector<ShaderResourceBinding> UIRenderPass::CollectResourceBindings() {
    ShaderResourceBinding uiTexSampler;
    uiTexSampler._id = 0;
    uiTexSampler._type = ShaderInputType::TEXTURE;
    uiTexSampler._shaderStage = ShaderStage::STAGE_FRAGMENT;
    uiTexSampler._identifier = "uiTexture";

    std::vector<ShaderResourceBinding> resourceBindings;
    resourceBindings.push_back(uiTexSampler);

    return resourceBindings;
}

std::string UIRenderPass::GetVertexShaderPath() {
    return COMBINE_SHADER_DIR(fullScreenQuadShader.vert);
}

std::string UIRenderPass::GetFragmentShaderPath() {
    return COMBINE_SHADER_DIR(fullScreenQuadShader.frag);
}

std::set<std::shared_ptr<Texture2D>> UIRenderPass::GetTextureResources(Scene *scene) {
    return {_texture};
}

void UIRenderPass::Process(Encoders encoders, Scene *scene, GraphicsPipeline *pipeline) {
    // When render returns false, or it failed or there is no need to update the gpu texture

    Shader* fs = _pipeline->GetFragmentShader();
    if(!fs) {
        assert(0);
        return;
    }


    if(!_ultralightRenderer->Render()) {
        ShaderInputResource uiTextureInputResource;
        uiTextureInputResource._binding = fs->GetShaderResourceBinding("uiTexture");
        uiTextureInputResource._textureResource._texture = _texture;

        encoders._renderEncoder->BindShaderResources(fs, {uiTextureInputResource});
        encoders._renderEncoder->Draw(6);

        return;
    }

    const void* pixelData = _ultralightRenderer->LockPixels();
    if(!pixelData) {
        _ultralightRenderer->UnlockPixels();
        assert(0);
        return;
    }

    // Update texture with pixel data
    if(TextureResource* resource = _texture->GetResource().get()) {
        void* data = resource->Lock();
        // TODO Validate if we are copy the correct amount of data, if we resize we might not
        std::memcpy(data, pixelData, _ultralightRenderer->PixelBufferSize());
        resource->Unlock();
    }

    _texture->MakeDirty();

    // Upload it to the GPU
//    encoders._renderEncoder->MakeImageBarrier(_texture.get(), ImageLayout::LAYOUT_TRANSFER_DST);
//    encoders._blitEncoder->UploadImageBuffer(_texture);
//    encoders._renderEncoder->MakeImageBarrier(_texture.get(), ImageLayout::LAYOUT_SHADER_READ);

    _ultralightRenderer->UnlockPixels();

    ShaderInputResource uiTextureInputResource;
    uiTextureInputResource._binding = fs->GetShaderResourceBinding("uiTexture");
    uiTextureInputResource._textureResource._texture = _texture;

    encoders._renderEncoder->BindShaderResources(fs, {uiTextureInputResource});
    encoders._renderEncoder->Draw(6);
}
