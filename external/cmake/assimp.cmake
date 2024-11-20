include(FetchContent)
message("Using ASSIMP")

FetchContent_Declare(assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG master)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
        
FetchContent_MakeAvailable(assimp)

target_link_libraries(${TARGET_NAME} PRIVATE assimp::assimp)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(${TARGET_NAME} PRIVATE -Wno-error)
