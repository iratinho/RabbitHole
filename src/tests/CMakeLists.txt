cmake_minimum_required(VERSION 3.24.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_NAME "TestsApplication")
set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)

project(
    ${PROJECT_NAME}
    VERSION 0.0.1
    DESCRIPTION ""
    LANGUAGES CXX)

project(${PROJECT_NAME} LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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
)

set(TEST_EXECUTABLE "TestApplication")
add_executable(${TEST_EXECUTABLE} src/dag.cpp src/renderGraph.cpp)

target_link_libraries(${TEST_EXECUTABLE} "Engine" GTest::gtest_main)
target_include_directories(${TEST_EXECUTABLE} PRIVATE ../engine/includes)
target_precompile_headers(${TEST_EXECUTABLE} PRIVATE ${PRECOMPLED_HEADERS})

include(GoogleTest)
gtest_discover_tests(${TEST_EXECUTABLE})


# include(CTest)
# add_test(NAME DAG COMMAND ${TEST_EXECUTABLE} "${CMAKE_CURRENT_LIST_DIR}")

# add_test(NAME DAG COMMAND ${PROJECT_NAME})

