cmake_minimum_required(VERSION 3.24.0)
include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)

set(SOURCES 
    src/main.cpp
)

set(INCLUDES)
set(TARGET_NAME "Application")

add_executable(${TARGET_NAME} ${SOURCES} ${INCLUDES})
target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPLED_HEADERS})
target_compile_definitions(${TARGET_NAME} PRIVATE "NDEBUG")
target_link_libraries(${TARGET_NAME} PRIVATE Engine)

if(APPLE)
    target_link_libraries(${TARGET_NAME}
            INTERFACE
            "-framework Cocoa"
    )
endif ()

add_common_target_properties(${TARGET_NAME})
add_precompiled_headers(${TARGET_NAME})

if (EMSCRIPTEN)
    set(ASSIMP_BUILD_ZLIB ON)

    target_link_options(${TARGET_NAME} PRIVATE
            -sUSE_GLFW=3 # Use Emscripten-provided GLFW
            -sUSE_WEBGPU # Handle WebGPU symbols
            -sASYNCIFY # Required by WebGPU-C++
    )

    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".html")
endif ()

set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/matcaps.cmake")
