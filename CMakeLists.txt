cmake_minimum_required(VERSION 3.24.0)

set_property(GLOBAL PROPERTY GLOBAL_DEPENDS_DEBUG_MODE 1)

set(PROJECT_NAME "RabbitHole")
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/out/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT EMSCRIPTEN) 
    set(CMAKE_OSX_ARCHITECTURES "arm64") # Invalid when compiling with emscripten
endif()

find_package(Python3 REQUIRED)

# execute script to download external libraries that are not submodules TODO Consider merging scripts
execute_process(COMMAND ${Python3_EXECUTABLE}  "./scripts/download_prerequisite.py" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} RESULT_VARIABLE outt)
# message(${outt})
execute_process(COMMAND ${Python3_EXECUTABLE} "./scripts/copy_webpack_artifacts.py" WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} RESULT_VARIABLE outt)
# message(${outt})

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)

message(${CMAKE_SYSTEM_PROCESSOR})


add_subdirectory(src/application)
add_subdirectory(src/engine)

#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#SET(CMAKE_INSTALL_RPATH "@loader_path/")


#enable_testing()
#add_subdirectory(src/tests)