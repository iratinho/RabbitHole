option(ASSIMP_SOURCE "Build with assimp sources" OFF)
OPTION ( ASSIMP_WARNINGS_AS_ERRORS "Treat all warnings as errors." OFF)
include(FetchContent)

if(ASSIMP_SOURCE)
    include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)

    message("Building with ASSIMP")
    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(${ASSIMP_WARNINGS_AS_ERRORS} OFF CACHE "" INTERNAL FORCE)
    add_subdirectory(${LIBRARIES_DIRECTORY}/assimp ${CMAKE_BINARY_DIR}/assimp)
    target_link_libraries(${TARGET_NAME} PRIVATE assimp)
else()
    message("Using ASSIMP")
    set(${ASSIMP_WARNINGS_AS_ERRORS} OFF CACHE BOOL "" INTERNAL)
    FetchContent_Declare(assimp
            GIT_REPOSITORY https://github.com/assimp/assimp.git
            GIT_TAG master)
    FetchContent_MakeAvailable(assimp)
    target_link_libraries(${TARGET_NAME} PRIVATE assimp::assimp)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_options(${TARGET_NAME} PRIVATE -Wno-error)
endif()