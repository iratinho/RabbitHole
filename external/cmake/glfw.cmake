option(GLFW_SOURCE "Build with glfw sources" OFF)
include(FetchContent)

# We need to find vulkan lib to specify where glfw will find the loader

if(GLFW_SOURCE)
    message("Building with GLFW")
    add_subdirectory(${LIBRARIES_DIRECTORY}/glfw ${CMAKE_BINARY_DIR}/glfw)
    target_link_libraries(${TARGET_NAME} PRIVATE glfw ${UNIX_LIBS})
    target_include_directories(${TARGET_NAME} PRIVATE ${LIBRARIES_DIRECTORY}/glfw/include)
    #target_compile_definitions(glfw PUBLIC _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")
else()
    message("Using GLFW")
    FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)
    FetchContent_MakeAvailable(glfw)

    target_link_libraries(${TARGET_NAME} PRIVATE glfw ${UNIX_LIBS})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    if(${USE_VULKAN})
        find_package(Vulkan)
        target_compile_definitions(${TARGET_NAME} PRIVATE _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")
    endif()
endif()

