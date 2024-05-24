message("Building with GLM")
add_subdirectory(${LIBRARIES_DIRECTORY}/glm ${CMAKE_BINARY_DIR}/glm)
target_link_libraries(${TARGET_NAME} PUBLIC glm)
