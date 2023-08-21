message("Building with GLFW")
add_subdirectory(${LIBRARIES_DIRECTORY}/glfw ${CMAKE_BINARY_DIR}/glfw)
target_link_libraries(${TARGET_NAME} PUBLIC glfw ${UNIX_LIBS})
target_include_directories(${TARGET_NAME} PUBLIC ${LIBRARIES_DIRECTORY}/glfw/include)
