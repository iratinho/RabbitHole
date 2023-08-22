option(GLFW_SOURCE "Build with glfw sources" OFF)

# We need to find vulkan lib to specify where glfw will find the loader
find_package(Vulkan)

if(GLFW_SOURCE)
    message("Building with GLFW")
    add_subdirectory(${LIBRARIES_DIRECTORY}/glfw ${CMAKE_BINARY_DIR}/glfw)
    target_link_libraries(${TARGET_NAME} PUBLIC glfw ${UNIX_LIBS})
    target_include_directories(${TARGET_NAME} PUBLIC ${LIBRARIES_DIRECTORY}/glfw/include)
    target_compile_definitions(glfw PUBLIC _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")
else()
    message("Using GLFW")
    execute_process(COMMAND ${CMAKE_COMMAND} --install . --config ${CMAKE_BUILD_TYPE} WORKING_DIRECTORY ${LIBRARIES_DIRECTORY}/.build/glfw RESULT_VARIABLE outt)
    find_package(glfw3 CONFIG REQUIRED PATHS ${CMAKE_BINARY_DIR}/generated_install/lib/cmake/glfw3/)
    target_link_libraries(${TARGET_NAME} PUBLIC glfw ${UNIX_LIBS})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(${TARGET_NAME} PUBLIC _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")
endif()

