message("Building with VULKAN")

# vulkan
find_package(Vulkan COMPONENTS shaderc_combined glslang)
target_include_directories(${TARGET_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(glfw PUBLIC _GLFW_VULKAN_LIBRARY="${Vulkan_LIBRARY}")

# vulkan shader compiler
# https://github.com/KhronosGroup/glslang
target_link_libraries(${TARGET_NAME} PUBLIC ${Vulkan_shaderc_combined_LIBRARY}) # glslang depends on shaderc_combined
target_link_libraries(${TARGET_NAME} PUBLIC ${Vulkan_glslang_LIBRARY})
