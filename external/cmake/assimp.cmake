option(ASSIMP_SOURCE "Build with assimp source" ON)

if(ASSIMP_SOURCE)
    message("Building with ASSIMP")
    set(ASSIMP_WARNINGS_AS_ERRORS OFF)
    add_subdirectory(${LIBRARIES_DIRECTORY}//assimp ${CMAKE_BINARY_DIR}/assimp)
    target_link_libraries(${TARGET_NAME} PRIVATE assimp)
endif()