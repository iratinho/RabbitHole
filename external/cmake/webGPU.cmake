include(FetchContent)

message("Building with webGPU")

if(EMSCRIPTEN)
    FetchContent_Declare(
        webgpu-backend-emscripten
        GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
        GIT_TAG        emscripten-v3.1.61
        GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(webgpu-backend-emscripten)
else ()
    FetchContent_Declare(
            webgpu-backend-wgpu
            GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
            GIT_TAG        wgpu-5433868
            GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(webgpu-backend-wgpu)
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE webgpu)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


