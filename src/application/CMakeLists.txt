cmake_minimum_required(VERSION 3.24.0)
include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)

option(USE_VULKAN "Build with vulkan support" FALSE)
option(WEBGPU_NATIVE "Builds with webgpu native" FALSE)

if (EMSCRIPTEN)
    # Disable Vulkan and WebGPU-native support for Emscripten
    set(USE_VULKAN OFF CACHE BOOL "Build with Vulkan support" FORCE)
    set(WEBGPU_NATIVE OFF CACHE BOOL "Build with WebGPU native" FORCE)

    message(STATUS "Emscripten WebGPU backend enabled.")
endif ()

if(USE_VULKAN AND WEBGPU_NATIVE)
    message(FATAL_ERROR "Both USE_VULKAN and WEBGPU-NATIVE are enabled. Please enable only one of them.")
endif()

if(USE_VULKAN)
    message(STATUS "Vulkan backend enabled.")
endif ()

if(WEBGPU_NATIVE)
    message(STATUS "Native WebGPU backend enabled.")
endif ()

if(NOT USE_VULKAN AND NOT WEBGPU_NATIVE AND NOT EMSCRIPTEN)
    message(FATAL_ERROR "No gpu backend selected.")
endif ()

set(SOURCES 
    src/main.cpp
)

set(INCLUDES)
set(TARGET_NAME "Application")

add_executable(${TARGET_NAME} ${SOURCES} ${INCLUDES})
target_precompile_headers(${TARGET_NAME} PRIVATE ${PRECOMPLED_HEADERS})
target_compile_definitions(${TARGET_NAME} PRIVATE "NDEBUG")
target_link_libraries(${TARGET_NAME} PRIVATE "Engine")
add_common_target_properties(${TARGET_NAME})
add_precompiled_headers(${TARGET_NAME})
#set_target_properties(${TARGET_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "@executable_path/  @rpath/ ${CMAKE_BINARY_DIR}/src/engine/$<CONFIG>/")

if(APPLE AND NOT EMSCRIPTEN)
    target_link_libraries(${TARGET_NAME}
            PRIVATE
            "-framework Cocoa"
            "-framework AppKit"
    )
endif ()

if (EMSCRIPTEN)
    set(ASSIMP_BUILD_ZLIB OFF)

    target_link_options(${TARGET_NAME} PRIVATE
            -sUSE_GLFW=3 # Use Emscripten-provided GLFW
            -sUSE_WEBGPU # Handle WebGPU symbols
            -sASYNCIFY # Required by WebGPU-C++
            -sWASM=1
            -sALLOW_MEMORY_GROWTH=1
            -O0
            -sSTACK_OVERFLOW_CHECK=1
            -g
            --preload-file assets
        )

    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".html")
endif ()

set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/matcaps.cmake")

if(WEBGPU_NATIVE)
    # Very special case because webgpu library is messing up the link paths for the build
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND /usr/bin/install_name_tool
        -change /Users/runner/work/wgpu-native/wgpu-native/target/aarch64-apple-darwin/release/deps/libwgpu_native.dylib
        @rpath/libwgpu_native.dylib
        $<TARGET_FILE:Application>
        COMMENT "Fixing library paths with install_name_tool"
    )
endif ()


