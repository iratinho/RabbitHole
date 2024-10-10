message("Building with glfw3webgpu")

include(${PROJECT_SOURCE_DIR}/CMakeCommon.cmake)

set(GLFW3_WEBGPU_TARGET "glfw3webgpu")
add_library(${GLFW3_WEBGPU_TARGET} STATIC ${LIBRARIES_DIRECTORY}/glfw3webgpu/glfw3webgpu.cpp)
target_include_directories(${GLFW3_WEBGPU_TARGET} PUBLIC ${LIBRARIES_DIRECTORY}/glfw3webgpu/)
target_link_libraries(${GLFW3_WEBGPU_TARGET} PUBLIC glfw webgpu)
add_common_target_properties(${GLFW3_WEBGPU_TARGET})

if (APPLE)
    target_compile_options(${GLFW3_WEBGPU_TARGET} PRIVATE -x objective-c)
    #target_link_libraries(${GLFW3_WEBGPU_TARGET} PRIVATE "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE ${GLFW3_WEBGPU_TARGET})
