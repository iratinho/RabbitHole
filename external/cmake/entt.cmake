message("Building with ENTT")
add_subdirectory(${LIBRARIES_DIRECTORY}/entt ${CMAKE_BINARY_DIR}/entt)
target_link_libraries(${TARGET_NAME} PRIVATE EnTT::EnTT)