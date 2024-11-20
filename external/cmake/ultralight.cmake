message("Building with ULTRALIGHT")

set(TARGET_NAME Application)

if (WIN32)
    set(ULTRALIGHT_LIB
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Ultralight.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UltralightCore.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/WebCore.lib
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AppCore.lib
    )
elseif (APPLE)
    set(ULTRALIGHT_LIB
            ${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/${CMAKE_BUILD_TYPE}/libUltralight.dylib
            ${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/${CMAKE_BUILD_TYPE}/libUltralightCore.dylib
            ${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/${CMAKE_BUILD_TYPE}/libWebCore.dylib
            ${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/${CMAKE_BUILD_TYPE}/libAppCore.dylib
    )
elseif (UNIX)
    set(ULTRALIGHT_LIB
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralight.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libUltralightCore.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libWebCore.so
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libAppCore.so
    )
endif ()

message(STATUS ${CMAKE_BUILD_TYPE})
target_link_libraries(${TARGET_NAME} PRIVATE ${ULTRALIGHT_LIB})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated_install/ultralight/include)

#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#SET(CMAKE_INSTALL_RPATH "@executable_path/")

# Copy Ultralight DLLs or dylibs to the output directory
add_custom_target(CopyUltralightLibraries
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Debug/>
        $<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Release/>
        COMMENT "Copying Ultralight libraries to output directory"
)

# Copy UI assets to the output directory
add_custom_target(CopyUltralightAssets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/assets
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Debug/assets/>
        $<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Release/assets/>
        COMMENT "Copying Ultralight assets to output directory"
)

# Copy resources to the output directory
add_custom_target(CopyUltralightResources
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/generated_install/ultralight/resources
        $<$<CONFIG:Debug>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Debug/assets/resources>
        $<$<CONFIG:Release>:${CMAKE_BINARY_DIR}/src/${TARGET_NAME}/Release/assets/resources>
        COMMENT "Copying Ultralight assets to output directory"
)

add_dependencies(${TARGET_NAME} CopyUltralightLibraries CopyUltralightAssets CopyUltralightResources)
set(TARGET_NAME Engine)
