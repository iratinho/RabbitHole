cmake_minimum_required(VERSION 3.24.0)

set(ENGINE_LIB Engine)

find_package(Python3 REQUIRED)

# execute script to download external libraries that are not submodules
execute_process(COMMAND ${Python3} ${PROJECT_SOURCE_DIR}/scripts/download_prerequisite.py RESULT_VARIABLE outt)

set(VULKAN_SOURCE_FILES
    "src/Renderer/Vendor/Vulkan/VkGraphicsPipeline.cpp"
    "src/Renderer/Vendor/Vulkan/VkGraphicsContext.cpp"
    "src/Renderer/Vendor/Vulkan/VKTransferContext.cpp"
    # "src/Renderer/Vendor/Vulkan/VkTexture2D.cpp"
    "src/Renderer/Vendor/Vulkan/VkTextureResource.cpp"
    "src/Renderer/Vendor/Vulkan/VkTextureView.cpp"
    "src/Renderer/Vendor/Vulkan/VKBuffer.cpp"
    "src/Renderer/Vendor/Vulkan/VKShader.cpp"
    "src/Renderer/Vendor/Vulkan/VKCommandEncoder.cpp"
    "src/Renderer/VulkanLoader.cpp"
    # "src/Renderer/Vendor/Vulkan/VKCommandQueue.cpp"
    # "src/Renderer/Vendor/Vulkan/VKCommandBuffer.cpp"
)

set(VULKAN_INCLUDE_FILES
    "includes/Renderer/Vendor/Vulkan/VkGraphicsPipeline.hpp"
    "includes/Renderer/Vendor/Vulkan/VKGraphicsContext.hpp"
    "includes/Renderer/Vendor/Vulkan/VKTransferContext.hpp"
    # "includes/Renderer/Vendor/Vulkan/VkTexture2D.hpp"
    "includes/Renderer/Vendor/Vulkan/VkTextureResource.hpp"
    "includes/Renderer/Vendor/Vulkan/VkTextureView.hpp"
    "includes/Renderer/Vendor/Vulkan/VKBuffer.hpp"
    "includes/Renderer/Vendor/Vulkan/VKShader.hpp"
    "includes/Renderer/Vendor/Vulkan/VKCommandEncoder.hpp"
    # "includes/Renderer/Vendor/Vulkan/VKCommandQueue.hpp"
    # "includes/Renderer/Vendor/Vulkan/VKCommandBuffer.hpp"
)

set(RENDERER_INCLUDES_FILES
    ${VULKAN_INCLUDE_FILES}

    "includes/Renderer/render_context.hpp"
    "includes/Renderer/RenderSystemV2.hpp"
    "includes/Renderer/ShaderCompiler.hpp"
    "includes/Renderer/GraphicsPipeline.hpp"
    "includes/Renderer/GraphicsContext.hpp"
    "includes/Renderer/TransferContext.hpp"
    "includes/Renderer/CommandEncoder.hpp"
    "includes/Renderer/Texture2D.hpp"
    "includes/Renderer/TextureResource.hpp"
    "includes/Renderer/TextureView.hpp"
    "includes/Renderer/RenderTarget.hpp"
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

    "includes/Renderer/Processors/MaterialProcessors.hpp"
    "includes/Renderer/Processors/TransformProcessor.hpp"
    "includes/Renderer/Processors/GeometryProcessors.hpp"
)

set(RENDERER_SOURCE_FILES
    ${VULKAN_SOURCE_FILES}
    
    "src/Renderer/render_context.cpp"
    "src/Renderer/RenderSystemV2.cpp"
    "src/Renderer/ShaderCompiler.cpp"
    "src/Renderer/GraphicsPipeline.cpp"
    "src/Renderer/GraphicsContext.cpp"
    "src/Renderer/TransferContext.cpp"
    "src/Renderer/CommandEncoder.cpp"
    "src/Renderer/Texture2D.cpp"
    "src/Renderer/TextureResource.cpp"
    "src/Renderer/TextureView.cpp"
    "src/Renderer/RenderTarget.cpp"
    "src/Renderer/Swapchain.cpp"
    "src/Renderer/Surface.cpp"
    "src/Renderer/Buffer.cpp"
    "src/Renderer/Fence.cpp"
    "src/Renderer/Shader.cpp"
    "src/Renderer/Material.cpp"
    "src/Renderer/GraphBuilder.cpp"

    #"src/Renderer/Processors/MaterialProcessors.cpp"
    #"src/Renderer/Processors/TransformProcessor.cpp"
)

set(SOURCE_FILES
    "src/application.cpp"
    "src/window.cpp"

    # "src/Renderer/RenderPass/FloorGridRenderPass.cpp"
    # "src/Renderer/RenderPass/OpaqueRenderer.cpp"
    # "src/Renderer/RenderPass/OpaqueRenderPass.cpp"
    # "src/Renderer/RenderPass/FullScreenQuadRenderPass.cpp"
    # "src/Renderer/CommandPool.cpp"
    # "src/Renderer/CommandBuffer.cpp"
    # "src/Renderer/RenderPass/RenderPassGenerator.cpp"
    # "src/Renderer/RenderPass/RenderPassExecutor.cpp"
    # "src/Renderer/FrameResources.cpp"
    # "src/Renderer/CommandQueue.cpp"
    # "src/Renderer/CommandBuffer.cpp"

    ${RENDERER_SOURCE_FILES}

    "src/Core/InputSystem.cpp"
    "src/Core/CameraSystem.cpp"
    "src/Core/ArcBallCamera.cpp"
    "src/Core/GeometryLoaderSystem.cpp"
    "src/Core/Scene.cpp"
    # "src/Core/Camera.cpp"
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
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")


