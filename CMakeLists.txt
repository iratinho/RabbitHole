cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(PROJECT_NAME "RabbitHole")

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)

# All Modules that our sub_projects will use
add_subdirectory(src/engine ${CMAKE_CURRENT_BINARY_DIR}/engine)

# Sub-Projects
add_subdirectory(src/application)

enable_testing()
add_subdirectory(src/tests)
