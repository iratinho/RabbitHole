cmake_minimum_required(VERSION 3.24.0)

set(PROJECT_NAME "ExternalLibs")
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/")

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)

set(TARGET_NAME ExternalLibsTarget)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET_NAME} "null.cpp")
target_compile_features(${TARGET_NAME} PRIVATE cxx_std_20)

include("cmake/libraries.cmake")
