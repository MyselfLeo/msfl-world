# Don't build docs
set(GLFW_BUILD_DOCS OFF)

# Don't build examples
set(GLFW_BUILD_EXAMPLES OFF)

# Don't build tests
set(GLFW_BUILD_TESTS OFF)

# Wayland only for Linux
set(GLFW_BUILD_X11 OFF)

# GLFW as static library
set(GLFW_LIBRARY_TYPE STATIC)

message("----- GLFW -----")

add_subdirectory(lib/glfw)
include_directories(lib/glfw/include)