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
target_include_directories(${APP_EXECUTABLE} PRIVATE ../engine/includes)
target_precompile_headers(${APP_EXECUTABLE} PRIVATE ${PRECOMPLED_HEADERS})
target_compile_definitions(${APP_EXECUTABLE} PRIVATE "NDEBUG")
target_link_libraries(${APP_EXECUTABLE} PRIVATE "Engine")

set_target_properties(${APP_EXECUTABLE} PROPERTIES LINK_FLAGS "-framework Cocoa")
# set_property (TARGET ${APP_EXECUTABLE}  APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")


set(TARGET_NAME ${APP_EXECUTABLE})
set(INCLUDE_TARGETS${APP_EXECUTABLE})
set(LIBRARIES_DIRECTORY "${CMAKE_SOURCE_DIR}/external")
include("${LIBRARIES_DIRECTORY}/cmake/matcaps.cmake")

