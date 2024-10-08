cmake_minimum_required(VERSION 3.24.0)

set(ENGINE_LIB Engine)

set(VULKAN_SOURCE_FILES
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
        "src/Renderer/VulkanLoader.cpp"
        "src/Renderer/Vendor/Vulkan/VKSwapchain.cpp"
        "src/Renderer/Vendor/Vulkan/VKWindow.cpp"
)

set(VULKAN_INCLUDE_FILES
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
)

set(RENDERER_INCLUDES_FILES
        ${VULKAN_INCLUDE_FILES}

        "includes/Renderer/RenderSystemV2.hpp"
        "includes/Renderer/ShaderCompiler.hpp"
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
        "includes/Renderer/VulkanLoader.hpp"
        "includes/Renderer/VulkanTranslator.hpp"
        "includes/Renderer/VulkanFunctions.inl"
        "includes/Renderer/Interfaces/TextureInterface.hpp"
        "includes/Renderer/Interfaces/RenderTargetInterface.hpp"
        "includes/Renderer/GPUDefinitions.h"

        # Command Encoders
        "includes/Renderer/CommandEncoders/GeneralCommandEncoder.hpp"
        "includes/Renderer/CommandEncoders/RenderCommandEncoder.hpp"
        "includes/Renderer/CommandEncoders/BlitCommandEncoder.hpp"

        # Processors
        "includes/Renderer/Processors/MaterialProcessors.hpp"
        "includes/Renderer/Processors/TransformProcessor.hpp"
        "includes/Renderer/Processors/GeometryProcessors.hpp"

        # Render Pass
        "includes/Renderer/RenderPass/RenderPassRegistration.inl"
        "includes/Renderer/RenderPass/RenderPassInterface.hpp"
        "includes/Renderer/RenderPass/FloorGridRenderPass.hpp"
        "includes/Renderer/RenderPass/PhongRenderPass.hpp"
        "includes/Renderer/RenderPass/MatcapRenderPass.hpp"
        "includes/Renderer/RenderPass/UIRenderPass.hpp"

        "includes/Renderer/Device.hpp"
        "includes/Renderer/Ultralight/UltralightRenderer.hpp"
)

set(RENDERER_SOURCE_FILES
        ${VULKAN_SOURCE_FILES}

        "src/Renderer/RenderSystemV2.cpp"
        "src/Renderer/ShaderCompiler.cpp"
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

        # Command Encoders
        "src/Renderer/CommandEncoders/GeneralCommandEncoder.cpp"
        "src/Renderer/CommandEncoders/RenderCommandEncoder.cpp"
        "src/Renderer/CommandEncoders/BlitCommandEncoder.cpp"

        # Processors
        "src/Renderer/Processors/MaterialProcessors.cpp"

        # Render Pass
        "src/Renderer/RenderPass/RenderPassInterface.cpp"
        "src/Renderer/RenderPass/FloorGridRenderPass.cpp"
        "src/Renderer/RenderPass/PhongRenderPass.cpp"
        "src/Renderer/RenderPass/MatcapRenderPass.cpp"
        "src/Renderer/RenderPass/UIRenderPass.cpp"

        "src/Renderer/Device.cpp"
        "src/Renderer/Ultralight/UltralightRenderer.cpp"
)

set(WINDOW_INCLUDE_FILES
        "includes/Window/Desktop/DesktopWindow.hpp"
)

set(WINDOW_SOURCE_FILES
        "src/Window/Desktop/DesktopWindow.cpp"
)

set(SOURCE_FILES
        "src/application.cpp"
        "src/window.cpp"

        ${RENDERER_SOURCE_FILES}
        ${WINDOW_SOURCE_FILES}

        "src/Core/InputSystem.cpp"
        "src/Core/CameraSystem.cpp"
        "src/Core/ArcBallCamera.cpp"
        "src/Core/GeometryLoaderSystem.cpp"
        "src/Core/Scene.cpp"
        "src/Core/Light.cpp"
        "src/Core/GenericFactory.cpp"
)

set(COMPONENTS_INCLUDE_FILES
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
)

set(INCLUDE_FILES
        ${RENDERER_INCLUDES_FILES}
        ${COMPONENTS_INCLUDE_FILES}
        ${WINDOW_INCLUDE_FILES}

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


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INCLUDE_FILES})

add_library(${ENGINE_LIB} SHARED ${SOURCE_FILES} ${INCLUDE_FILES})
target_compile_features(${ENGINE_LIB} PRIVATE cxx_std_20)
target_include_directories(${ENGINE_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_precompile_headers(${ENGINE_LIB} PRIVATE ${PRECOMPLED_HEADERS})

if (UNIX)
    list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread -lstdc++)
    target_link_libraries(${ENGINE_LIB} PUBLIC ${UNIX_LIBS})
endif (UNIX)

# Add a macro with shaders location
target_compile_definitions(${ENGINE_LIB} PUBLIC VK_SHADER_DIR=${CMAKE_SOURCE_DIR}/src/engine/shaders/)


# set_target_properties(${ENGINE_LIB}  PROPERTIES LINK_FLAGS "-framework Cocoa -framework Metal -framework Foundation -framework CoreFoundation -framework QuartzCore -framework MetalKit")
# set_property (TARGET ${ENGINE_LIB}  APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")


# Set up RPATH for dynamic libraries on macOS
# if (APPLE)
#     set_target_properties(${TARGET_NAME} PROPERTIES
#             BUILD_WITH_INSTALL_RPATH TRUE
#             INSTALL_RPATH "@loader_path/"
#             INSTALL_RPATH_USE_LINK_PATH TRUE
#     )
# endif ()


# add_compile_options(-fsanitize=address)
# add_link_options(-fsanitize=address)

# External Libraries
set(TARGET_NAME ${ENGINE_LIB})
set(INCLUDE_TARGETS${ENGINE_LIB})
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/fmt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/glm.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/glfw.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/vulkan.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/entt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/ultralight.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/assimp.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/stb.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/grpc.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/delegates.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")
