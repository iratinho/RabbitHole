set( _fmt_HEADER_SEARCH_DIRS "${CMAKE_SOURCE_DIR}/.build/generated_install/fmt/include")
set( _fmt_LIB_SEARCH_DIRS "${CMAKE_SOURCE_DIR}/.build/generated_install/fmt/lib" )

# Search for the header
find_path(GLFW3_INCLUDE_DIR "GLFW/glfw3.h"
PATHS ${_glfw3_HEADER_SEARCH_DIRS} )

# Search for the library
find_library(GLFW3_LIBRARY NAMES glfw3 glfw
PATHS ${_glfw3_LIB_SEARCH_DIRS} )
INCLUDE(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW3 DEFAULT_MSG GLFW3_LIBRARY GLFW3_INCLUDE_DIR)