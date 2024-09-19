cmake_minimum_required(VERSION 3.24.0)

set(ENGINE_LIB Engine)

find_package(Python3 REQUIRED)

# execute script to download external libraries that are not submodules
execute_process(COMMAND ${Python3} ${PROJECT_SOURCE_DIR}/scripts/download_prerequisite.py RESULT_VARIABLE outt)

set(VULKAN_SOURCE_FILES
    "src/Renderer/Vendor/Vulkan/VKGraphicsPipeline.cpp"
    "src/Renderer/Vendor/Vulkan/VKGraphicsContext.cpp"
    # "src/Renderer/Vendor/Vulkan/VkTexture2D.cpp"
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

        # "src/Renderer/Vendor/Vulkan/VKCommandQueue.cpp"
)

set(VULKAN_INCLUDE_FILES
    "includes/Renderer/Vendor/Vulkan/VkGraphicsPipeline.hpp"
    "includes/Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
    # "includes/Renderer/Vendor/Vulkan/VkTexture2D.hpp"
    "includes/Renderer/Vendor/Vulkan/VkTextureResource.hpp"
    "includes/Renderer/Vendor/Vulkan/VkTextureView.hpp"
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
    # "includes/Renderer/Vendor/Vulkan/VKCommandQueue.hpp"
    "includes/Renderer/Vendor/Vulkan/VKDevice.hpp"
    "includes/Renderer/Vendor/Vulkan/VkSwapchain.hpp"
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
    #"includes/Renderer/RenderTarget.hpp"
    "includes/Renderer/Swapchain.hpp"
    "includes/Renderer/Surface.hpp"
    "includes/Renderer/Buffer.hpp"
    "includes/Renderer/Fence.hpp"
    "includes/Renderer/Shader.hpp"
    "includes/Renderer/Material.hpp"
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

    "includes/Renderer/Device.hpp"
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
    #"src/Renderer/RenderTarget.cpp"
    "src/Renderer/Swapchain.cpp"
    "src/Renderer/Surface.cpp"
    "src/Renderer/Buffer.cpp"
    "src/Renderer/Fence.cpp"
    "src/Renderer/Shader.cpp"
    "src/Renderer/Material.cpp"
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

    "src/Renderer/Device.cpp"
)

set(SOURCE_FILES
    "src/application.cpp"
    "src/window.cpp"

    ${RENDERER_SOURCE_FILES}

    "src/Core/InputSystem.cpp"
    "src/Core/CameraSystem.cpp"
    "src/Core/ArcBallCamera.cpp"
    "src/Core/GeometryLoaderSystem.cpp"
    "src/Core/Scene.cpp"
    "src/Core/MeshObject.cpp"
    "src/Core/Light.cpp"
    "src/Core/GenericFactory.cpp"

    "src/UI/UISystem.cpp"
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
    # "includes/Renderer/RenderPass/FloorGridRenderPass.hpp"
    # "includes/Renderer/RenderPass/OpaqueRenderer.hpp"
    # "includes/Renderer/RenderPass/OpaqueRenderPass.hpp"
    # "includes/Renderer/RenderPass/FullScreenQuadRenderPass.hpp"
    # "includes/Renderer/CommandPool.hpp"
    # "includes/Renderer/CommandBuffer.hpp"
    # "includes/Renderer/RenderPass/RenderPassGenerator.hpp"
    # "includes/Renderer/RenderPass/RenderPassExecutor.hpp"
    # "includes/Renderer/CommandQueue.hpp"
    # "includes/Renderer/CommandBuffer.hpp"

    ${RENDERER_INCLUDES_FILES}
    ${COMPONENTS_INCLUDE_FILES}

    "includes/Core/Utils.hpp"
    "includes/Core/InputSystem.hpp"
    "includes/Core/CameraSystem.hpp"
    "includes/Core/GeometryLoaderSystem.hpp"
    "includes/Core/Scene.hpp"
    # "includes/Core/Camera.hpp"
    "includes/Core/ArcBallCamera.hpp"
    "includes/Core/IBaseObject.hpp"
    "includes/Core/GenericInstanceWrapper.hpp"
    "includes/Core/MeshObject.hpp"
    "includes/Core/Light.hpp"
    "includes/Core/GenericFactory.hpp"
    "includes/Core/Cache/Cache.hpp"
    "includes/Core/Containers/ObjectPool.hpp"
    
    "includes/UI/UISystem.hpp"

    "includes/application.hpp"
)

set(PRECOMPLED_HEADERS
    "<cstdint>"
    "<iostream>"
    "<vector>"
    "<unordered_set>"
    "<algorithm>"
    "<tuple>"
    "<bitset>"
    "<cstring>"
    "<fstream>"
    "<sstream>"
    "<array>"
    "<memory>"
    "<string>"
    "<ranges>"
    "<any>"
    "<utility>"
    "<queue>"
    "<functional>"
    "<map>"
    "<assert.h>"
    "<mutex>"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INCLUDE_FILES})

add_library(${ENGINE_LIB} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})
target_compile_features(${ENGINE_LIB} PRIVATE cxx_std_20)
target_include_directories(${ENGINE_LIB} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_precompile_headers(${ENGINE_LIB} PRIVATE ${PRECOMPLED_HEADERS})

if (UNIX)
 list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread -lstdc++)
 target_link_libraries(${ENGINE_LIB} PUBLIC ${UNIX_LIBS})
endif (UNIX)

# Add a macro with shaders location
target_compile_definitions(${ENGINE_LIB} PUBLIC VK_SHADER_DIR=${CMAKE_SOURCE_DIR}/src/engine/shaders/)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

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
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")
