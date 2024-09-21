message("Building with ULTRALIGHT")

if (WIN32)
    set(ULTRALIGHT_LIB
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/Ultralight.lib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/UltralightCore.lib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/WebCore.lib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/lib/AppCore.lib
    )
elseif (APPLE)
    set(ULTRALIGHT_LIB
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralight.dylib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralightCore.dylib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libWebCore.dylib
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libAppCore.dylib
    )
elseif (UNIX)
    set(ULTRALIGHT_LIB
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralight.so
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libUltralightCore.so
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libWebCore.so
            ${CMAKE_BINARY_DIR}/generated_install/ultralight/bin/libAppCore.so
    )
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE ${ULTRALIGHT_LIB})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/generated_install/ultralight/include)

# Copy ultralight dlls
#add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        #COMMAND ${CMAKE_COMMAND} -E copy_directory
        #${CMAKE_BINARY_DIR}/generated_install/ultralight/bin
        #$<TARGET_FILE_DIR:${TARGET_NAME}>)

