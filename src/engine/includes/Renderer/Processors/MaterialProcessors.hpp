#pragma once
#include "Components/DirectionalLightComponent.hpp"
#include "Components/PhongMaterialComponent.hpp"
#include "Components/GridMaterialComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Renderer/GraphicsPipeline.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/RenderTarget.hpp"
#include "Renderer/Shader.hpp"
#include "Core/Scene.hpp"

class RenderContext;
class CommandEncoder;
class Scene;

#ifndef STR_EXPAND
#define STR_EXPAND(tok) #tok
#endif

#ifndef STR
#define STR(tok) STR_EXPAND(tok)
#endif

#ifndef COMBINE_SHADER_DIR
#define COMBINE_SHADER_DIR(name) STR(VK_SHADER_DIR) "/" STR(name)
#endif

// https://stackoverflow.com/questions/51210197/class-specialization-involving-crtp-and-inner-type

template <typename Child>
class BaseMaterialProcessor {
public:
    static void GenerateShaders(GraphicsContext* graphicsContext) {
        Child::BuildImp(graphicsContext);
    };
    
    template <typename Entity>
    static void Process(GraphicsContext* graphicsContext, CommandEncoder* encoder, GraphicsPipeline* pipeline, Scene* scene, Entity entity) {
        Child::template ProcessImp<Entity>(graphicsContext, encoder, pipeline, scene, entity);
    };
    
    static std::shared_ptr<Shader> GetVertexShader(GraphicsContext* graphicsContext) {
        return Child::GetVertexShaderImp(graphicsContext);
    };
    
    static std::shared_ptr<Shader> GetFragmentShader(GraphicsContext* graphicsContext) {
        return Child::GetFragmentShaderImp(graphicsContext);
    };
};

template <typename T>
class MaterialProcessor : public BaseMaterialProcessor<MaterialProcessor<T>> {};

template<>
class MaterialProcessor<PhongMaterialComponent> : public BaseMaterialProcessor<MaterialProcessor<PhongMaterialComponent>> {
    using Base = MaterialProcessor<PhongMaterialComponent>;
    friend BaseMaterialProcessor<Base>;
private:
    static void BuildImp(GraphicsContext* graphicsContext) {
        ShaderInputBinding vertexDataBinding;
        vertexDataBinding._binding = 0;
        vertexDataBinding._stride = sizeof(VertexData);

        // Position vertex input
        ShaderInputLocation positions;
        positions._format = Format::FORMAT_R32G32B32_SFLOAT;
        positions._offset = offsetof(VertexData, position);
        
        ShaderInputLocation normals;
        normals._format = Format::FORMAT_R32G32B32_SFLOAT;
        normals._offset = offsetof(VertexData, normal);
        
        auto vertexShader = Base::GetVertexShaderImp(graphicsContext);
        vertexShader->DeclareShaderBindingLayout(vertexDataBinding, { positions, normals });
        vertexShader->DeclarePushConstant<glm::mat4>("mvp_matrix");
        vertexShader->DeclarePushConstant<glm::vec4>("lightColor");
        vertexShader->DeclarePushConstant<glm::vec4>("lightDirection");
        vertexShader->DeclarePushConstant<float>("lightIntensity");
        vertexShader->DeclarePushConstant<glm::vec3>("cameraPosition");

        auto fragmentShader = Base::GetFragmentShaderImp(graphicsContext);

        fragmentShader->DeclareShaderOutput("");
    };
    
    template <typename Entity>
    static void ProcessImp(GraphicsContext* graphicsContext, CommandEncoder* encoder, GraphicsPipeline* pipeline, Scene* scene, Entity entity) {
        // We should have a viewport abstraction that would know this type of information
        int width = graphicsContext->GetSwapchainColorTarget()->GetWidth();
        int height = graphicsContext->GetSwapchainColorTarget()->GetHeight();
                
        // Camera
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        glm::vec3 cameraPosition;
        const auto cameraView = scene->GetRegistry().view<TransformComponent, CameraComponent>();
        for(auto cameraEntity : cameraView) {
            auto [transformComponent, cameraComponent] = cameraView.get<TransformComponent, CameraComponent>(cameraEntity);
            viewMatrix = cameraComponent.m_ViewMatrix;
            projMatrix = glm::perspective(cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 180.f);
            
            encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "cameraPosition", &transformComponent.m_Position);
            
            break;
        }
        
        // Lights
        const auto directionalLightView = scene->GetRegistry().view<DirectionalLightComponent>();
        for (auto lightEntity : directionalLightView) {
            auto& lightComponent = directionalLightView.get<DirectionalLightComponent>(lightEntity);
            
            encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "lightDirection", &lightComponent._direction);
            encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "lightColor", &lightComponent._color);
            encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "lightIntensity", &lightComponent._intensity);

            break;
        }

        auto view = scene->GetRegistry().view<TransformComponent>();
        const auto& transform = view.get<TransformComponent>(entity);
        
        glm::mat4 mvp = projMatrix * viewMatrix * transform._computedMatrix.value();
        encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "mvp_matrix", &mvp);
    }
    
    static std::shared_ptr<Shader> GetVertexShaderImp(GraphicsContext* graphicsContext) {
        return Shader::MakeShader(graphicsContext, COMBINE_SHADER_DIR(dummy.vert), ShaderStage::STAGE_VERTEX);
    };
    
    static std::shared_ptr<Shader> GetFragmentShaderImp(GraphicsContext* graphicsContext) {
        return Shader::MakeShader(graphicsContext, COMBINE_SHADER_DIR(dummy.frag), ShaderStage::STAGE_FRAGMENT);
    };
};

template <>
class MaterialProcessor<GridMaterialComponent> : public BaseMaterialProcessor<MaterialProcessor<GridMaterialComponent>> {
    using Base = MaterialProcessor<GridMaterialComponent>;
    friend BaseMaterialProcessor<Base>;
private:
    static void BuildImp(GraphicsContext* graphicsContext) {
        // Binding 0 for vertex data
        ShaderInputBinding vertexDataBinding;
        vertexDataBinding._binding = 0;
        vertexDataBinding._stride = sizeof(VertexData);
        
        // Position vertex input
        ShaderInputLocation positions;
        positions._format = Format::FORMAT_R32G32B32_SFLOAT;
        positions._offset = offsetof(VertexData, position);

        auto vertexShader = Base::GetVertexShaderImp(graphicsContext);
        vertexShader->DeclareShaderBindingLayout(vertexDataBinding, { positions });
        vertexShader->DeclarePushConstant<glm::mat4>("viewMatrix");
        vertexShader->DeclarePushConstant<glm::mat4>("projMatrix");
        
        auto fragmentShader = Base::GetFragmentShaderImp(graphicsContext);
        fragmentShader->DeclareShaderOutput("");
    };
    
    template <typename Entity>
    static void ProcessImp(GraphicsContext* graphicsContext, CommandEncoder* encoder, GraphicsPipeline* pipeline, Scene* scene, Entity entity) {
        // We should have a viewport abstraction that would know this type of information
        int width = graphicsContext->GetSwapchainColorTarget()->GetWidth();
        int height = graphicsContext->GetSwapchainColorTarget()->GetHeight();

        // Extract the viewMatrix from the camera
        const auto cameraView = scene->GetRegistry().view<CameraComponent>();
        glm::mat4 viewMatrix;
        glm::mat4 projMatrix;
        if(cameraView.size() > 0) {
            const auto& cameraComponent = cameraView.get<CameraComponent>(cameraView[0]);
            viewMatrix = cameraComponent.m_ViewMatrix;
            projMatrix = glm::perspective(
                cameraComponent.m_Fov, ((float)width / (float)height), 0.1f, 180.f);
        }
        
        encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "viewMatrix", &viewMatrix);
        encoder->UpdatePushConstant(graphicsContext, pipeline, Base::GetVertexShaderImp(graphicsContext).get(), "projMatrix", &projMatrix);
    };
    
    static std::shared_ptr<Shader> GetVertexShaderImp(GraphicsContext* graphicsContext) {
        return Shader::MakeShader(graphicsContext, COMBINE_SHADER_DIR(floor_grid.vert), ShaderStage::STAGE_VERTEX);
    };
    
    static std::shared_ptr<Shader> GetFragmentShaderImp(GraphicsContext* graphicsContext) {
        return Shader::MakeShader(graphicsContext, COMBINE_SHADER_DIR(floor_grid.frag), ShaderStage::STAGE_FRAGMENT);
    };

};
