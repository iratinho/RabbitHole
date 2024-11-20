if(${USE_VULKAN})
    list(APPEND SOURCE_FILES
            "src/Renderer/Vendor/Vulkan/VKGraphicsPipeline.cpp"
            "src/Renderer/Vendor/Vulkan/VKGraphicsContext.cpp"
            "src/Renderer/Vendor/Vulkan/VkTextureResource.cpp"
            "src/Renderer/Vendor/Vulkan/VkTextureView.cpp"
            "src/Renderer/Vendor/Vulkan/VKBuffer.cpp"
            "src/Renderer/Vendor/Vulkan/VKImageBuffer.cpp"
            "src/Renderer/Vendor/Vulkan/VKShader.cpp"
            "src/Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.cpp"
            "src/Renderer/Vendor/Vulkan/VKRenderCommandEncoder.cpp"
            "src/Renderer/Vendor/Vulkan/VKBlitCommandEncoder.cpp"
            "src/Renderer/Vendor/Vulkan/VKCommandBuffer.cpp"
            "src/Renderer/Vendor/Vulkan/VKEvent.cpp"
            "src/Renderer/Vendor/Vulkan/VKFence.cpp"
            "src/Renderer/Vendor/Vulkan/VKDescriptorSetsManager.cpp"
            "src/Renderer/Vendor/Vulkan/VKDescriptorPool.cpp"
            "src/Renderer/Vendor/Vulkan/VKSamplerManager.cpp"
            "src/Renderer/Vendor/Vulkan/VKDevice.cpp"
            "src/Renderer/Vendor/Vulkan/VulkanLoader.cpp"
            "src/Renderer/Vendor/Vulkan/VKSwapchain.cpp"
            "src/Renderer/Vendor/Vulkan/VKWindow.cpp"
            "src/Renderer/Vendor/Vulkan/ShaderCompiler.cpp"
    )

    list(APPEND INCLUDE_FILES
            "includes/Renderer/Vendor/Vulkan/VkGraphicsPipeline.hpp"
            "includes/Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
            "includes/Renderer/Vendor/Vulkan/VkTextureResource.hpp"
            "includes/Renderer/Vendor/Vulkan/VKTextureView.hpp"
            "includes/Renderer/Vendor/Vulkan/VKBuffer.hpp"
            "includes/Renderer/Vendor/Vulkan/VKImageBuffer.hpp"
            "includes/Renderer/Vendor/Vulkan/VKShader.hpp"
            "includes/Renderer/Vendor/Vulkan/VKRenderCommandEncoder.hpp"
            "includes/Renderer/Vendor/Vulkan/VKBlitCommandEncoder.hpp"
            "includes/Renderer/Vendor/Vulkan/VKGeneralCommandEncoder.hpp"
            "includes/Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
            "includes/Renderer/Vendor/Vulkan/VKEvent.hpp"
            "includes/Renderer/Vendor/Vulkan/VKFence.hpp"
            "includes/Renderer/Vendor/Vulkan/VKDescriptorSetsManager.hpp"
            "includes/Renderer/Vendor/Vulkan/VKDescriptorPool.hpp"
            "includes/Renderer/Vendor/Vulkan/VKSamplerManager.hpp"
            "includes/Renderer/Vendor/Vulkan/VKDevice.hpp"
            "includes/Renderer/Vendor/Vulkan/VkSwapchain.hpp"
            "includes/Renderer/Vendor/Vulkan/VKWindow.hpp"
            "includes/Renderer/Vendor/Vulkan/VulkanLoader.hpp"
            "includes/Renderer/Vendor/Vulkan/VulkanTranslator.hpp"
            "includes/Renderer/Vendor/Vulkan/VulkanFunctions.inl"
            "includes/Renderer/Vendor/Vulkan/ShaderCompiler.hpp"
    )
endif ()

list(APPEND SOURCE_FILES
        "src/Window/Desktop/DesktopWindow.cpp"

        "src/Renderer/RenderSystemV2.cpp"
        "src/Renderer/GraphicsPipeline.cpp"
        "src/Renderer/GraphicsContext.cpp"
        "src/Renderer/CommandBuffer.cpp"
        "src/Renderer/Event.cpp"
        "src/Renderer/Texture2D.cpp"
        "src/Renderer/TextureResource.cpp"
        "src/Renderer/TextureView.cpp"
        "src/Renderer/Swapchain.cpp"
        "src/Renderer/Buffer.cpp"
        "src/Renderer/Fence.cpp"
        "src/Renderer/Shader.cpp"
        "src/Renderer/GraphBuilder.cpp"
        "src/Renderer/CommandEncoders/GeneralCommandEncoder.cpp"
        "src/Renderer/CommandEncoders/RenderCommandEncoder.cpp"
        "src/Renderer/CommandEncoders/BlitCommandEncoder.cpp"
        "src/Renderer/Processors/MaterialProcessors.cpp"
        "src/Renderer/RenderPass/RenderPassInterface.cpp"
        "src/Renderer/RenderPass/FloorGridRenderPass.cpp"
        "src/Renderer/RenderPass/PhongRenderPass.cpp"
        "src/Renderer/RenderPass/MatcapRenderPass.cpp"
        #"src/Renderer/RenderPass/UIRenderPass.cpp"
        "src/Renderer/Device.cpp"
        #"src/Renderer/Ultralight/UltralightRenderer.cpp"

        "src/Core/InputSystem.cpp"
        "src/Core/CameraSystem.cpp"
        "src/Core/ArcBallCamera.cpp"
        "src/Core/GeometryLoaderSystem.cpp"
        "src/Core/Scene.cpp"
        "src/Core/Light.cpp"
        "src/Core/GenericFactory.cpp"

        "src/application.cpp"
        "src/window.cpp"
)

list(APPEND INCLUDE_FILES
        "includes/Components/TransformComponent.hpp"
        "includes/Components/CameraComponent.hpp"
        "includes/Components/InputComponent.hpp"
        "includes/Components/UserInterfaceComponent.hpp"
        "includes/Components/MeshComponent.hpp"
        "includes/Components/DirectionalLightComponent.hpp"
        "includes/Components/MaterialComponent.hpp"
        "includes/Components/PrimitiveComponent.hpp"
        "includes/Components/PrimitiveProxyComponent.hpp"
        "includes/Components/GridMaterialComponent.hpp"
        "includes/Components/MatCapMaterialComponent.hpp"

        "includes/Window/Desktop/DesktopWindow.hpp"

        "includes/Renderer/RenderSystemV2.hpp"
        "includes/Renderer/GraphicsPipeline.hpp"
        "includes/Renderer/GraphicsContext.hpp"
        "includes/Renderer/CommandBuffer.hpp"
        "includes/Renderer/Event.hpp"
        "includes/Renderer/Texture2D.hpp"
        "includes/Renderer/TextureResource.hpp"
        "includes/Renderer/TextureView.hpp"
        "includes/Renderer/Swapchain.hpp"
        "includes/Renderer/Buffer.hpp"
        "includes/Renderer/Fence.hpp"
        "includes/Renderer/Shader.hpp"
        "includes/Renderer/GraphBuilder.hpp"
        "includes/Renderer/FrameResources.hpp"
        "includes/Renderer/Interfaces/TextureInterface.hpp"
        "includes/Renderer/Interfaces/RenderTargetInterface.hpp"
        "includes/Renderer/GPUDefinitions.h"
        "includes/Renderer/CommandEncoders/GeneralCommandEncoder.hpp"
        "includes/Renderer/CommandEncoders/RenderCommandEncoder.hpp"
        "includes/Renderer/CommandEncoders/BlitCommandEncoder.hpp"
        "includes/Renderer/Processors/MaterialProcessors.hpp"
        "includes/Renderer/Processors/TransformProcessor.hpp"
        "includes/Renderer/Processors/GeometryProcessors.hpp"
        "includes/Renderer/RenderPass/RenderPassRegistration.inl"
        "includes/Renderer/RenderPass/RenderPassInterface.hpp"
        "includes/Renderer/RenderPass/FloorGridRenderPass.hpp"
        "includes/Renderer/RenderPass/PhongRenderPass.hpp"
        "includes/Renderer/RenderPass/MatcapRenderPass.hpp"
        #"includes/Renderer/RenderPass/UIRenderPass.hpp"
        "includes/Renderer/Device.hpp"
        #"includes/Renderer/Ultralight/UltralightRenderer.hpp"

        "includes/Core/Utils.hpp"
        "includes/Core/InputSystem.hpp"
        "includes/Core/CameraSystem.hpp"
        "includes/Core/GeometryLoaderSystem.hpp"
        "includes/Core/Scene.hpp"
        "includes/Core/ArcBallCamera.hpp"
        "includes/Core/IBaseObject.hpp"
        "includes/Core/GenericInstanceWrapper.hpp"
        "includes/Core/Light.hpp"
        "includes/Core/GenericFactory.hpp"
        "includes/Core/Cache/Cache.hpp"
        "includes/Core/Containers/ObjectPool.hpp"
        "includes/window.hpp"
        "includes/application.hpp"
)

if(WEBGPU_NATIVE OR EMSCRIPTEN)
    list(APPEND SOURCE_FILES
            "src/Renderer/Vendor/WebGPU/WebGPUTranslate.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUDevice.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUWindow.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUCommandBuffer.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUSwapchain.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUTextureView.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUTextureResource.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUGraphicsContext.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUPipeline.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUShader.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUBuffer.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUBlitCommandEncoder.cpp"
            "src/Renderer/Vendor/WebGPU/WebGPUTextureBuffer.cpp"
    )

    list(APPEND INCLUDE_FILES
            "includes/Renderer/Vendor/WebGPU/WebGPUTranslate.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUDevice.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUWindow.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUCommandBuffer.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPURenderCommandEncoder.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUSwapchain.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUTextureView.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUTextureResource.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUGraphicsContext.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUPipeline.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUShader.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUBuffer.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUBlitCommandEncoder.hpp"
            "includes/Renderer/Vendor/WebGPU/WebGPUTextureBuffer.hpp"
    )
endif ()

list(APPEND SHADER_FILES
        # "shaders/matcap.frag"
        # "shaders/matcap.vert"
        # "shaders/floor_grid.frag"
        # "shaders/floor_grid.vert"
)
