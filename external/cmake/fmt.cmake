message("Building with FMT")
add_subdirectory(${LIBRARIES_DIRECTORY}/fmt ${CMAKE_BINARY_DIR}/fmt)
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt)
