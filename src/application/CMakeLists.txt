cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME "Application")

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)


set(SOURCES 
    src/main.cpp
)

set(INCLUDES)

set(APP_EXECUTABLE "Application")
add_executable(${APP_EXECUTABLE} ${SOURCES} ${INCLUDES})
target_include_directories(${APP_EXECUTABLE} PRIVATE includes)

target_link_libraries(${APP_EXECUTABLE} PRIVATE "Engine")
target_include_directories(${APP_EXECUTABLE} PRIVATE ../engine/includes)

