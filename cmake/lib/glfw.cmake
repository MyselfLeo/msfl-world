# Don't build docs
set(GLFW_BUILD_DOCS OFF)

# Don't build examples
set(GLFW_BUILD_EXAMPLES OFF)

# Don't build tests
set(GLFW_BUILD_TESTS OFF)

# Wayland only for Linux
if(MSFLW_ENABLE_X11_SUPPORT)
    set(GLFW_BUILD_X11 ON)
else()
    set(GLFW_BUILD_X11 OFF)
endif()

# GLFW as static library
set(GLFW_LIBRARY_TYPE STATIC)

message("----- GLFW -----")

add_subdirectory(lib/glfw)
target_include_directories(msfl_world PUBLIC lib/glfw/include)
target_link_libraries(msfl_world PUBLIC glfw)