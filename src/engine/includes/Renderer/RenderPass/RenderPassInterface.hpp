#pragma once

class GraphicsContext;
class GraphBuilder;
class Scene;

#define REGISTER_RENDER_PASS(PassType) \
    namespace { \
        static PassType PassType; \
        struct PassType##Registrar { \
            PassType##Registrar() { \
                RenderSystemV2::RegisterRenderPass(&PassType); \
            } \
        }; \
        static PassType##Registrar global_##PassType##Registrar; \
    }

class IRenderPass {
public:
    virtual ~IRenderPass() = default;
    virtual bool Setup(GraphBuilder* graphBuilder, GraphicsContext* graphicsContext, Scene* scene) = 0;
};
