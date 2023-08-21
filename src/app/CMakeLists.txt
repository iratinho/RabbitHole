cmake_minimum_required(VERSION 3.24.0)

set(TARGET_NAME Application)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 REQUIRED)

# execute script to download external libraries that are not submodules
execute_process(COMMAND ${Python3} ${PROJECT_SOURCE_DIR}/scripts/download_prerequisite.py RESULT_VARIABLE outt)

set(SOURCE_FILES
    "src/main.cpp"
    "src/application.cpp"
    "src/window.cpp"

    "src/Renderer/render_context.cpp"
    "src/Renderer/simple_rendering.cpp"
    "src/Renderer/RenderPass/FloorGridRenderPass.cpp"
    "src/Renderer/RenderPass/OpaqueRenderer.cpp"
    "src/Renderer/RenderPass/OpaqueRenderPass.cpp"
    "src/Renderer/RenderPass/FullScreenQuadRenderPass.cpp"
    "src/Renderer/RenderPass/RenderPass.cpp"
    "src/Renderer/RenderSystem.cpp"
    "src/Renderer/RenderTarget.cpp"
    "src/Renderer/Texture.cpp"
    "src/Renderer/RenderGraph/RenderGraph.cpp"
    "src/Renderer/RenderGraph/GraphBuilder.cpp"
    "src/Renderer/RenderGraph/Actions/RenderPassAction.cpp"
    "src/Renderer/RenderGraph/Actions/FenceAction.cpp"
    "src/Renderer/RenderGraph/Actions/SwapchainAction.cpp"
    "src/Renderer/RenderGraph/Actions/CommandPoolAction.cpp"
    "src/Renderer/RenderGraph/Actions/SurfaceAction.cpp"
    "src/Renderer/RenderGraph/Actions/BufferAction.cpp"
    "src/Renderer/VulkanLoader.cpp"
    "src/Renderer/Fence.cpp"
    "src/Renderer/Swapchain.cpp"
    "src/Renderer/CommandPool.cpp"
    "src/Renderer/CommandBuffer.cpp"
    "src/Renderer/Surface.cpp"
    "src/Renderer/Buffer.cpp"
    "src/Renderer/RenderPass/RenderPassGenerator.cpp"
    "src/Renderer/RenderPass/RenderPassExecutor.cpp"
    "src/Renderer/ShaderCompiler.cpp"

    "src/Core/InputSystem.cpp"
    "src/Core/CameraSystem.cpp"
    "src/Core/ArcBallCamera.cpp"
    "src/Core/GeometryLoaderSystem.cpp"
    "src/Core/Scene.cpp"
    "src/Core/Camera.cpp"
    "src/Core/MeshObject.cpp"
    "src/Core/Light.cpp"

    "src/UI/UISystem.cpp"
)

set(INCLUDE_FILES
    "includes/Core/Utils.hpp"
    "includes/Renderer/render_context.hpp"
    "includes/Renderer/simple_rendering.hpp"
    "includes/Renderer/RenderPass/FloorGridRenderPass.hpp"
    "includes/Renderer/RenderPass/OpaqueRenderer.hpp"
    "includes/Renderer/RenderPass/OpaqueRenderPass.hpp"
    "includes/Renderer/RenderPass/FullScreenQuadRenderPass.hpp"
    "includes/Renderer/RenderPass/RenderPass.hpp"
    "includes/Renderer/RenderSystem.hpp"
    "includes/Renderer/RenderTarget.hpp"
    "includes/Renderer/Texture.hpp"
    "includes/Renderer/RenderGraph/RenderGraph.hpp"
    "includes/Renderer/RenderGraph/GraphBuilder.hpp"
    "includes/Renderer/RenderGraph/Actions/RenderPassAction.hpp"
    "includes/Renderer/RenderGraph/Actions/FenceAction.hpp"
    "includes/Renderer/RenderGraph/Actions/SwapchainAction.hpp"
    "includes/Renderer/RenderGraph/Actions/CommandPoolAction.hpp"
    "includes/Renderer/RenderGraph/Actions/SurfaceAction.hpp"
    "includes/Renderer/RenderGraph/Actions/BufferAction.hpp"
    "includes/Renderer/VulkanLoader.hpp"
    "includes/Renderer/Fence.hpp"
    "includes/Renderer/Swapchain.hpp"
    "includes/Renderer/CommandPool.hpp"
    "includes/Renderer/CommandBuffer.hpp"
    "includes/Renderer/Surface.hpp"
    "includes/Renderer/Buffer.hpp"
    "includes/Renderer/VulkanFunctions.inl"
    "includes/Renderer/Interfaces/TextureInterface.hpp"
    "includes/Renderer/Interfaces/RenderTargetInterface.hpp"
    "includes/Renderer/GPUDefinitions.h"
    "includes/Renderer/RenderPass/RenderPassGenerator.hpp"
    "includes/Renderer/VulkanTranslator.hpp" 
    "includes/Renderer/RenderPass/RenderPassExecutor.hpp" 
    "includes/Renderer/ShaderCompiler.hpp"

    "includes/Core/Components/TransformComponent.hpp"
    "includes/Core/Components/CameraComponent.hpp"
    "includes/Core/Components/InputComponent.hpp"
    "includes/Core/Components/UserInterfaceComponent.hpp"
    "includes/Core/Components/MeshComponent.hpp"
    "includes/Core/Components/DirectionalLightComponent.hpp"
    "includes/Core/InputSystem.hpp"
    "includes/Core/CameraSystem.hpp"
    "includes/Core/GeometryLoaderSystem.hpp"
    "includes/Core/Scene.hpp"
    "includes/Core/Camera.hpp"
    "includes/Core/ArcBallCamera.hpp"
    "includes/Core/IBaseObject.hpp"
    "includes/Core/GenericInstanceWrapper.hpp"
    "includes/Core/MeshObject.hpp"
    "includes/Core/Light.hpp"
    
    "includes/UI/UISystem.hpp"
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
    "<fmt/core.h>"
    "<memory>"
    "<string>"
    "<ranges>"
    "<any>"
)

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPLED_HEADERS})

if (UNIX)
 list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread -lstdc++)
 target_link_libraries(${TARGET_NAME} PUBLIC ${UNIX_LIBS})
endif (UNIX)

# External Libraries
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")

# Add a macro with shaders location
target_compile_definitions(${TARGET_NAME} PUBLIC VK_SHADER_BYTE_CODE_DIR=${CMAKE_SOURCE_DIR}/src/app/shaders/)

