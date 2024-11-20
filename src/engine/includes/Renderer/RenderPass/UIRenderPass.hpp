#pragma once
#include "Renderer/RenderPass/RenderPassInterface.hpp"
#include "Renderer/Texture2D.hpp"
#include "Renderer/Ultralight/UltralightRenderer.hpp"

class UIRenderPass : public RenderPass {
public:
    void Initialize(GraphicsContext *graphicsContext) override;

    [[nodiscard]] std::string GetIdentifier() override;

    [[nodiscard]] RenderAttachments GetRenderAttachments(GraphicsContext *graphicsContext) override;

    [[nodiscard]] GraphicsPipelineParams GetPipelineParams() override;

    [[nodiscard]] ShaderInputBindings CollectShaderInputBindings() override;

    [[nodiscard]] std::vector<PushConstant> CollectPushConstants() override;

    [[nodiscard]] std::vector<ShaderResourceBinding> CollectResourceBindings() override;

    [[nodiscard]] std::string GetVertexShaderPath() override;

    [[nodiscard]] std::string GetFragmentShaderPath() override;

    [[nodiscard]] std::set<std::shared_ptr<Texture2D>> GetTextureResources(Scene *scene) override;

    void Process(GraphicsContext* graphicsContext, Encoders encoders, Scene *scene, GraphicsPipeline *pipeline) override;

private:
    // This should be unique
    std::shared_ptr<Texture2D> _texture;
    std::unique_ptr<UltralightRenderer> _ultralightRenderer;
};
