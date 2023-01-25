cmake_minimum_required(VERSION 3.16.0)

set(TARGET_NAME Application)

set(SOURCE_FILES
    "src/main.cpp"
    "src/application.cpp"
    "src/window.cpp"
    "src/Renderer/render_context.cpp"
    "src/Renderer/simple_rendering.cpp"
    "src/Renderer/FloorGridRenderer.cpp"
    "src/Renderer/OpaqueRenderer.cpp"
    "src/Renderer/RenderSystem.cpp"
    "src/Renderer/RenderTarget.cpp"
    "src/Renderer/Texture.cpp"
    "src/Renderer/RenderGraph/RenderGraph.cpp"
    "src/Renderer/RenderGraph/GraphBuilder.cpp"
    "src/Renderer/RenderPass.cpp"
    "src/Renderer/OpaqueRenderPass.cpp"
    "includes/render_context.h"
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
    "<format>"
)

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)
set(CMAKE_CXX_STANDARD 23)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPLED_HEADERS})

if (UNIX)
 list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread)
endif (UNIX)

# glfw
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/generated_install/glfw -B build -S . WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/glfw RESULT_VARIABLE outt)
execute_process(COMMAND ${CMAKE_COMMAND} --build build --target install WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/glfw RESULT_VARIABLE outt)
find_package(GLFW3 REQUIRED)
target_link_libraries(${TARGET_NAME} PUBLIC ${GLFW3_LIBRARY} ${UNIX_LIBS})
target_include_directories(${TARGET_NAME} PUBLIC ${GLFW3_INCLUDE_DIR})

# glm
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/glm/glm)

# vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(${TARGET_NAME} PUBLIC Vulkan::Vulkan)
target_include_directories(${TARGET_NAME} PUBLIC Vulkan::Headers)