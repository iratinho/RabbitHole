include(FetchContent)

message("Building with webGPU")

function(WebGPU_link_library target)
    if(EMSCRIPTEN)
        FetchContent_Declare(
                webgpu-backend-emscripten
                GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
                GIT_TAG        fa0b54d68841fb33188403b07959d403b24511de # emscripten-v3.1.61 + fix
                GIT_SHALLOW    FALSE
        )

        FetchContent_MakeAvailable(webgpu-backend-emscripten)
    else ()
        FetchContent_Declare(
                webgpu-backend-wgpu
                GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
                GIT_TAG        54a60379a9d792848a2311856375ceef16db150e # wgpu-v0.19.4.1 + fix
                GIT_SHALLOW    FALSE
        )
        # set(ARCH ${CMAKE_OSX_ARCHITECTURES}) # see wegpu ARCH.. this will fail on windows
        FetchContent_MakeAvailable(webgpu-backend-wgpu)
    endif ()

    target_link_libraries(${target} PUBLIC webgpu)
endfunction()

function(WebGPU_includes target)
    target_include_directories(${target} PRIVATE ${CMAKE_BINARY_DIR}/_deps/webgpu-backend-wgpu-src/include/)
endfunction()

function(WebGPU_copy_binaries target)
    add_custom_target(CopyWebGPUBinaries
            COMMAND ${CMAKE_COMMAND} -E echo "Copying WebGPU binary..."
            COMMAND ${CMAKE_COMMAND} -E copy
            ${WGPU_RUNTIME_LIB}
            $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Debug/libwgpu_native.dylib>
            $<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Release/libwgpu_native.dylib>
            COMMENT "Copying WebGPU binary"
    )

    add_dependencies(${TARGET_NAME} CopyWebGPUBinaries)

    #target_copy_webgpu_binaries(${target})

endfunction()






