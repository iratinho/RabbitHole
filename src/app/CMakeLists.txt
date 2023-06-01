cmake_minimum_required(VERSION 3.24.0)

set(TARGET_NAME Application)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OPTIONS
option(GLFW_PRECOMPILED "Use a precompiled version of glfw library" OFF)
option(GLFW_PRECOMPIED_VERSION "Glfw precompied version to download when using GLFW_PRECOMPILED" "3.3.8")
option(ASSIMP_PRECOMPILED "Use a precompiled version of assimp library" ON)
option(ASSIMP_PRECOMPIED_VERSION "Assimp precompied version to download when using ASSIMP_PRECOMPILED" "v5.2.5")

find_package(Python3 REQUIRED)

# execute script to download external libraries that are not submodules
execute_process(COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/download_prerequisite.py RESULT_VARIABLE outt)

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

    "src/Core/InputSystem.cpp"
    "src/Core/CameraSystem.cpp"
    "src/Core/GeometryLoaderSystem.cpp"

    "src/UI/UISystem.cpp"

    "src/Renderer/RenderPass/RenderPassGenerator.cpp"
        src/Renderer/RenderPass/RenderPassExecutor.cpp)

set(INCLUDE_FILES
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

    "includes/Core/Components/TransformComponent.hpp"
    "includes/Core/Components/CameraComponent.hpp"
    "includes/Core/Components/InputComponent.hpp"
    "includes/Core/Components/UserInterfaceComponent.hpp"
    "includes/Core/Components/MeshComponent.hpp"
    
    "includes/Core/InputSystem.hpp"
    "includes/Core/CameraSystem.hpp"
    "includes/Core/GeometryLoaderSystem.hpp"

    "includes/UI/UISystem.hpp"
    "includes/Renderer/GPUDefinitions.h"
    "includes/Renderer/RenderPass/RenderPassGenerator.hpp"
        includes/Renderer/VulkanTranslator.hpp includes/Renderer/RenderPass/RenderPassExecutor.hpp includes/Core/Utils.hpp)

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

# fmt
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/generated_install/fmt -B build -S . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/fmt RESULT_VARIABLE outt)
execute_process(COMMAND ${CMAKE_COMMAND} --build build --target install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/fmt RESULT_VARIABLE outt)
set(fmt_DIR ${CMAKE_SOURCE_DIR}/external/fmt/build/)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt )

# glfw
add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw ${CMAKE_BINARY_DIR}/glfw)
target_link_libraries(${TARGET_NAME} PUBLIC glfw ${UNIX_LIBS})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/glfw/include)

# glm
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/glm/glm)

# vulkan
find_package(Vulkan)
target_include_directories(${TARGET_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(glfw PUBLIC _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")

# Add a macro with shaders location
target_compile_definitions(${TARGET_NAME} PUBLIC VK_SHADER_BYTE_CODE_DIR=${CMAKE_SOURCE_DIR}/src/app/shaders/bytecode)

# entt
add_subdirectory(${CMAKE_SOURCE_DIR}/external/entt ${CMAKE_BINARY_DIR}/entt)
target_link_libraries(${TARGET_NAME} PUBLIC EnTT::EnTT)

# ultralight
if(WIN32)
    set(ULTRALIGHT_LIB
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/Ultralight.lib
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/UltralightCore.lib
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/WebCore.lib
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/AppCore.lib
    )
elseif(APPLE)
    set(ULTRALIGHT_LIB
        libUltralight.dylib
        libUltralightCore.dylib
        libWebCore.dylib
        libAppCore.dylib
    )
elseif(UNIX)
    set(ULTRALIGHT_LIB
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralight.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralightCore.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libWebCore.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libAppCore.so
    )
endif()

# Apple silicon is not yet supported, skip the linking for APPLE. Support will happen with v1.4 of ultralight
if(!APPLE)
    target_link_libraries(${TARGET_NAME} PUBLIC ${ULTRALIGHT_LIB})
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_BINARY_DIR}/generated_install/ultralight/include)

    # Copy ultralight dlls
    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin
            $<TARGET_FILE_DIR:${TARGET_NAME}>)
endif()

# assimp
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/assimp ${CMAKE_BINARY_DIR}/assimp)
target_link_libraries(${TARGET_NAME} PRIVATE assimp)
