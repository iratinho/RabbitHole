cmake_minimum_required(VERSION 3.24.0)
option(USE_VULKAN "Build with vulkan support" TRUE)

include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeSources.cmake)

set(TARGET_NAME Engine)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${INCLUDE_FILES})

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${INCLUDE_FILES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_definitions(${TARGET_NAME} PUBLIC VK_SHADER_DIR=${CMAKE_SOURCE_DIR}/src/engine/shaders/)
add_precompiled_headers(${TARGET_NAME})
add_common_target_properties(${TARGET_NAME})

#if (UNIX)
#    list(APPEND UNIX_LIBS ${CMAKE_DL_LIBS} pthread -lstdc++)
#    target_link_libraries(${TARGET_NAME} PUBLIC ${UNIX_LIBS})
#endif (UNIX)

# External Libraries
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/fmt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/glm.cmake")

if (NOT EMSCRIPTEN)
    include("${LIBRARIES_DIRECTORY}/cmake/ultralight.cmake")

    # We only want to include glfw when not using emscripten, it will provide its own version
    include("${LIBRARIES_DIRECTORY}/cmake/glfw.cmake")

    if(${USE_VULKAN})
        include("${LIBRARIES_DIRECTORY}/cmake/vulkan.cmake")
    endif()
endif ()

IF(EMSCRIPTEN)
    include("${LIBRARIES_DIRECTORY}/cmake/glfw3webgpu.cmake")
endif ()

include("${LIBRARIES_DIRECTORY}/cmake/webGPU.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/entt.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/assimp.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/stb.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/grpc.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/delegates.cmake")
include("${LIBRARIES_DIRECTORY}/cmake/libraries.cmake")
