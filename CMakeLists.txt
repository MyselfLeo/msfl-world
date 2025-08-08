cmake_minimum_required(VERSION 3.31)
project(msfl_world)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib/glfw-3.4)

set(GLM_STATIC_LIBRARY_ENABLE True)
add_subdirectory(lib/glm)

include_directories(lib/glad/include lib/glfw-3.4/include lib/glm lib/stb_image)
include_directories(src)

add_executable(msfl_world

        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp

        # Source code
        src/main.cpp
        src/Program.hpp
        src/Program.cpp
        src/Texture.hpp
        src/Texture.cpp
)

target_link_libraries(msfl_world glfw glm_static)
