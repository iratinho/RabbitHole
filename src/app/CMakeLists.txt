cmake_minimum_required(VERSION 3.24.0)

set(TARGET_NAME Application)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OPTIONS
option(GLFW_PRECOMPILED "Use a precompiled version of glfw library" OFF)
option(GLFW_PRECOMPIED_VERSION "Glfw precompied version to download when using GLFW_PRECOMPILED" "3.3.8")
option(ASSIMP_PRECOMPILED "Use a precompiled version of assimp library" ON)
option(ASSIMP_PRECOMPIED_VERSION "Assimp precompied version to download when using ASSIMP_PRECOMPILED" "v5.2.5")

find_package(Python REQUIRED)

# execute script to download external libraries that are not submodules
execute_process(COMMAND ${Python_EXECUTABLE} scripts/download_prerequisite.py RESULT_VARIABLE outt)

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
    "src/Renderer/VulkanLoader.cpp"
    "src/Renderer/Fence.cpp"
    "src/Renderer/Swapchain.cpp"
    "src/Renderer/CommandPool.cpp"
    "src/Renderer/CommandBufferManager.cpp"

    "src/Core/InputSystem.cpp"
    "src/Core/CameraSystem.cpp"

    "src/UI/UISystem.cpp"
)

set(INCLUDE_FILES
    "includes/Renderer/render_context.h"
    "includes/Renderer/simple_rendering.h"
    "includes/Renderer/RenderPass/FloorGridRenderPass.h"
    "includes/Renderer/RenderPass/OpaqueRenderer.h"
    "includes/Renderer/RenderPass/OpaqueRenderPass.h"
    "includes/Renderer/RenderPass/FullScreenQuadRenderPass.h"
    "includes/Renderer/RenderPass/RenderPass.h"
    "includes/Renderer/RenderSystem.h"
    "includes/Renderer/RenderTarget.h"
    "includes/Renderer/Texture.h"
    "includes/Renderer/RenderGraph/RenderGraph.h"
    "includes/Renderer/RenderGraph/GraphBuilder.h"
    "includes/Renderer/VulkanLoader.h"
    "includes/Renderer/Fence.h"
    "includes/Renderer/Swapchain.h"
    "includes/Renderer/CommandPool.h"
    "includes/Renderer/CommandBufferManager.h"
    "includes/Renderer/VulkanFunctions.inl"
    "includes/Renderer/Interfaces/TextureInterface.h"
    "includes/Renderer/Interfaces/RenderTargetInterface.h"

    "includes/Core/Components/TransformComponent.h"
    "includes/Core/Components/CameraComponent.h"
    "includes/Core/Components/InputComponent.h"
    "includes/Core/Components/UserInterfaceComponent.h"
    
    "includes/Core/InputSystem.h"
    "includes/Core/CameraSystem.h"

    "includes/UI/UISystem.h"
)

SOURCE_GROUP(Sources FILES ${SOURCE_FILES})
SOURCE_GROUP(Includes FILES ${INCLUDE_FILES})


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
)

file(GLOB HEADERS *.h)
source_group("Headers" FILES ${HEADERS})

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
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/generated_install/vulkan -B build -S . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/vkHeaders RESULT_VARIABLE outt)
execute_process(COMMAND ${CMAKE_COMMAND} --build build --target install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/vkHeaders RESULT_VARIABLE outt)
set(VulkanHeaders_DIR ${CMAKE_BINARY_DIR}/generated_install/vulkan/share/cmake/VulkanHeaders)
find_package(VulkanHeaders REQUIRED CONFIG)
target_link_libraries(${TARGET_NAME} PRIVATE Vulkan::Headers)

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
elseif(UNIX)
    set(ULTRALIGHT_LIB
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralight.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralightCore.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libWebCore.so
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libAppCore.so
    )
endif()

target_link_libraries(${TARGET_NAME} PUBLIC ${ULTRALIGHT_LIB})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_BINARY_DIR}/generated_install/ultralight/include)

# Copy ultralight dlls
add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin
        $<TARGET_FILE_DIR:${TARGET_NAME}>)

# assimp
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/assimp ${CMAKE_BINARY_DIR}/assimp)
target_link_libraries(${TARGET_NAME} PRIVATE assimp)

# Vulkan Validation layers
# execute_process(COMMAND ${CMAKE_COMMAND} -S . -B build -D UPDATE_DEPS=ON -D BUILD_WERROR=ON -D BUILD_TESTS=OFF -D CMAKE_BUILD_TYPE=Debug WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/Vulkan-ValidationLayers RESULT_VARIABLE outt)
# execute_process(COMMAND ${CMAKE_COMMAND} --build . --config Debug WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/Vulkan-ValidationLayers RESULT_VARIABLE outt)
