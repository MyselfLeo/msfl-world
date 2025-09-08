cmake_minimum_required(VERSION 3.31)
project(msfl_world)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(lib/glfw-3.4)
include_directories(lib/glfw-3.4/include)

set(GLM_STATIC_LIBRARY_ENABLE True)
add_subdirectory(lib/glm)
include_directories(lib/glm)

set(BUILD_SHARED_LIBS False)
set(ASSIMP_BUILD_ZLIB On)
set(ASSIMP_BUILD_TESTS Off)
set(ASSIMP_INSTALL Off)
set(ASSIMP_INJECT_DEBUG_POSTFIX Off)
set(ASSIMP_BUILD_ASSIMP_VIEW Off)
add_subdirectory(lib/assimp-6.0.2)
include_directories(lib/assimp-6.0.2/include)

add_subdirectory(lib/imgui-1.92.2b)
include_directories(lib/imgui-1.92.2b)
include_directories(lib/imgui-1.92.2b/backends)

include_directories(lib/glad/include lib/stb_image wrld)

add_library(msfl_world STATIC

        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp

        # Source code
        wrld/resources/Program.hpp
        wrld/resources/Program.cpp
        wrld/resources/Texture.hpp
        wrld/resources/Texture.cpp
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
        wrld/resources/Material.hpp
        wrld/resources/Material.cpp
        wrld/resources/WindowViewport.hpp
        wrld/resources/WindowViewport.cpp
        wrld/components/Orbiter.hpp
        wrld/components/Orbiter.cpp
        wrld/resources/Resource.cpp
        wrld/resources/Resource.hpp
        wrld/resources/Mesh.cpp
        wrld/resources/Mesh.hpp
        wrld/builtins.cpp
        wrld/builtins.hpp
)

target_link_libraries(msfl_world glfw glm assimp imgui imgui_impl_glfw imgui_impl_opengl3)

add_executable(main main.cpp)
target_link_libraries(main msfl_world)

add_executable(shader_test shader_test.cpp)
target_link_libraries(shader_test msfl_world)