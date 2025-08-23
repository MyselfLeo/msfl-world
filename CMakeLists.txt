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
include_directories(wrld)

add_executable(msfl_world

        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp

        # Source code
        wrld/main.cpp
        wrld/resources/Program.hpp
        wrld/resources/Program.cpp
        wrld/resources/Texture.hpp
        wrld/resources/Texture.cpp
        wrld/Camera.cpp
        wrld/Camera.hpp
        wrld/components/Component.cpp
        wrld/components/Component.hpp
        wrld/World.cpp
        wrld/World.hpp
        wrld/components/Transform.cpp
        wrld/components/Transform.hpp
        wrld/components/StaticModel.cpp
        wrld/components/StaticModel.hpp
        wrld/components/Camera.cpp
        wrld/components/Camera.hpp
        wrld/resources/Model.cpp
        wrld/resources/Model.hpp
        wrld/systems/RendererSystem.cpp
        wrld/systems/RendererSystem.hpp
        wrld/System.cpp
        wrld/System.hpp
        wrld/Logs.hpp
        wrld/components/PointLight.cpp
        wrld/components/PointLight.hpp
        wrld/components/DirectionalLight.cpp
        wrld/components/DirectionalLight.hpp
        wrld/components/FPSControl.cpp
        wrld/components/FPSControl.hpp
        wrld/resources/CubemapTexture.cpp
        wrld/resources/CubemapTexture.hpp
        wrld/components/Environment.cpp
        wrld/components/Environment.hpp
)

target_link_libraries(msfl_world glfw glm assimp)