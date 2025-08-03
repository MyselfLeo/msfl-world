cmake_minimum_required(VERSION 3.31)
project(msfl_world)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib/glfw-3.4)

include_directories(lib/glad/include lib/glfw-3.4/include)
include_directories(src)

add_executable(msfl_world
        lib/glad/src/glad.c
        src/main.cpp
)

target_link_libraries(msfl_world glfw)
