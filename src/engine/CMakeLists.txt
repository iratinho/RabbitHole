cmake_minimum_required(VERSION 3.24.0)

include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeSources.cmake)

set(TARGET_NAME "Engine")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INCLUDE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SHADER_FILES})

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES} ${SHADER_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_definitions(${TARGET_NAME} PUBLIC VK_SHADER_DIR=${CMAKE_SOURCE_DIR}/src/engine/shaders/)
add_precompiled_headers(${TARGET_NAME})
add_common_target_properties(${TARGET_NAME})

if(WEBGPU_NATIVE OR EMSCRIPTEN)
    target_compile_definitions(${TARGET_NAME} PUBLIC WEBGPU_BACKEND)
else ()
    target_compile_definitions(${TARGET_NAME} PUBLIC VULKAN_BACKEND)
endif ()

#if(APPLE AND NOT EMSCRIPTEN)
#    target_link_libraries(${TARGET_NAME}
#            PUBLIC
#            "-framework Cocoa"
#    )
#endif ()

#if (UNIX)
#    list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread -lstdc++)
#    target_link_libraries(${TARGET_NAME} PUBLIC ${UNIX_LIBS})
#endif (UNIX)

# External Libraries
include("${LIBRARIES_DIRECTORY}/cmake/fmt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/glm.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/entt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/assimp.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/stb.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/grpc.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/delegates.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")

if (NOT EMSCRIPTEN)
    #include("${LIBRARIES_DIRECTORY}/cmake/ultralight.cmake")
endif ()

if(WEBGPU_NATIVE OR EMSCRIPTEN)
    include("${LIBRARIES_DIRECTORY}/cmake/glfw3webgpu.cmake")
    include("${LIBRARIES_DIRECTORY}/cmake/webGPU.cmake")
    WebGPU_includes(${TARGET_NAME})
    WebGPU_link_library(${TARGET_NAME})

    if(WEBGPU_NATIVE)
        WebGPU_copy_binaries(${TARGET_NAME})
    endif ()
endif()


if(NOT EMSCRIPTEN)
    # We only want to include glfw when not using emscripten, emscripten will provide its own version
    include("${LIBRARIES_DIRECTORY}/cmake/glfw.cmake")
endif()

if(${USE_VULKAN})
    include("${LIBRARIES_DIRECTORY}/cmake/vulkan.cmake")
endif()
