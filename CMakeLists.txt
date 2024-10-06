cmake_minimum_required(VERSION 3.24.0)

find_package(Python3 REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

# execute script to download external libraries that are not submodules TODO Consider merging scripts
execute_process(COMMAND ${Python3_EXECUTABLE}  "./scripts/download_prerequisite.py" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} RESULT_VARIABLE outt)
message(${outt})
execute_process(COMMAND ${Python3_EXECUTABLE} "./scripts/copy_webpack_artifacts.py" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} RESULT_VARIABLE outt)
message(${outt})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME "RabbitHole")

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)

    # Set output directories for all configurations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# For multi-configuration generators
# Handle multi-configuration generators (Xcode, Visual Studio)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/out)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/out)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/out)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/out)

SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "@loader_path/")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/out/)

set(PRECOMPLED_HEADERS
        "<cstdint>"
        "<iostream>"
        "<vector>"
        "<unordered_set>"
        "<algorithm>"
        "<tuple>"
        "<bitset>"
        "<cstring>"
        "<fstream>"
        "<sstream>"
        "<array>"
        "<memory>"
        "<string>"
        "<ranges>"
        "<any>"
        "<utility>"
        "<queue>"
        "<functional>"
        "<map>"
        "<assert.h>"
        "<mutex>"
)

# All Modules that our sub_projects will use
add_subdirectory(src/engine ${CMAKE_CURRENT_BINARY_DIR}/engine)

# Sub-Projects
add_subdirectory(src/application)

enable_testing()
add_subdirectory(src/tests)