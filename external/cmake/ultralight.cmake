message("Building with ULTRALIGHT")

if (WIN32)
    set(ULTRALIGHT_LIB
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Ultralight.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UltralightCore.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/WebCore.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AppCore.lib
    )
elseif (APPLE)
    set(ULTRALIGHT_LIB
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralight.dylib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralightCore.dylib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libWebCore.dylib
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libAppCore.dylib
    
    )
elseif (UNIX)
    set(ULTRALIGHT_LIB
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralight.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralightCore.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libWebCore.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libAppCore.so
    )
endif ()

# Link Ultralight libraries
target_link_libraries(${TARGET_NAME} PRIVATE ${ULTRALIGHT_LIB})

# Include Ultralight headers
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated_install/ultralight/include)

# Copy Ultralight DLLs or dylibs to the output directory
add_custom_target(CopyUltralightLibraries ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Copying Ultralight libraries to output directory"
)

# Copy UI assets to the output directory
add_custom_target(CopyUltralightAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/assets
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
        COMMENT "Copying Ultralight assets to output directory"
)

# Copy resources to the output directory
add_custom_target(CopyUltralightResources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/resources
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/resources"
        COMMENT "Copying Ultralight assets to output directory"
)

#
add_custom_target(UltralightDeps "" DEPENDS CopyUltralightLibraries CopyUltralightAssets CopyUltralightResources)
add_dependencies(${TARGET_NAME} UltralightDeps)

