# Use Glew as a static library
set(GLEW_USE_STATIC_LIBS ON)

# No tools
set(BUILD_UTILS OFF)

# Only libs
set(ONLY_LIBS ON)

set(GLEW_EGL ON)

if(MSFLW_ENABLE_X11_SUPPORT)
    set(GLEW_X11 ON)
else()
    set(GLEW_X11 OFF)
    target_compile_definitions(msfl_world PUBLIC -DGLEW_EGL -DGLEW_STATIC)
endif()

message("----- GLEW -----")

add_subdirectory(
        lib/glew-cmake/build/cmake/
        lib/glew-cmake
)
target_include_directories(msfl_world PUBLIC lib/glew-cmake/include)
target_link_libraries(msfl_world PUBLIC glew_s)