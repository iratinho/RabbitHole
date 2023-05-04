cmake_minimum_required(VERSION 3.24.0)

set(TARGET_NAME Application)

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
)

file(GLOB HEADERS *.h)
source_group("Headers" FILES ${HEADERS})

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPLED_HEADERS})

if (UNIX)
 list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread)
endif (UNIX)

# fmt
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/generated_install/fmt -B build -S . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/fmt RESULT_VARIABLE outt)
execute_process(COMMAND ${CMAKE_COMMAND} --build build --target install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/fmt RESULT_VARIABLE outt)
set(fmt_DIR ${CMAKE_SOURCE_DIR}/external/fmt/build/)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt)

# glfw
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/generated_install/glfw -B build -S . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/glfw RESULT_VARIABLE outt)
execute_process(COMMAND ${CMAKE_COMMAND} --build build --target install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/glfw RESULT_VARIABLE outt)
find_package(GLFW3 REQUIRED)
target_link_libraries(${TARGET_NAME} PUBLIC ${GLFW3_LIBRARY} ${UNIX_LIBS})
target_include_directories(${TARGET_NAME} PUBLIC ${GLFW3_INCLUDE_DIR})

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
set(ULTRALIGHT_LIB
    ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/Ultralight.lib
    ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/UltralightCore.lib
    ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/WebCore.lib
    ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/AppCore.lib
)

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
