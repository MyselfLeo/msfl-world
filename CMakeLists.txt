cmake_minimum_required(VERSION 3.31)
project(msfl_world)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib/glfw-3.4)

set(GLM_STATIC_LIBRARY_ENABLE True)
add_subdirectory(lib/glm)

set(BUILD_SHARED_LIBS False)
set(ASSIMP_BUILD_ZLIB On)
set(ASSIMP_BUILD_TESTS Off)
set(ASSIMP_INSTALL Off)
set(ASSIMP_INJECT_DEBUG_POSTFIX Off)
set(ASSIMP_BUILD_ASSIMP_VIEW Off)
add_subdirectory(lib/assimp-6.0.2)

include_directories(lib/glad/include lib/glfw-3.4/include lib/glm lib/stb_image lib/assimp-6.0.2/include)
include_directories(src)

add_executable(msfl_world

        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp

        # Source code
        src/main.cpp
        src/resources/Program.hpp
        src/resources/Program.cpp
        src/resources/Texture.hpp
        src/resources/Texture.cpp
        src/Camera.cpp
        src/Camera.hpp
        src/Component.cpp
        src/Component.hpp
        src/World.cpp
        src/World.hpp
        src/components/TransformComponent.cpp
        src/components/TransformComponent.hpp
        src/components/ModelComponent.cpp
        src/components/ModelComponent.hpp
        src/components/CameraComponent.cpp
        src/components/CameraComponent.hpp
        src/resources/Model.cpp
        src/resources/Model.hpp
        src/systems/RendererSystem.cpp
        src/systems/RendererSystem.hpp
        src/System.cpp
        src/System.hpp
)

target_link_libraries(msfl_world glfw glm_static assimp)
