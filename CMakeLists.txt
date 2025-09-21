cmake_minimum_required(VERSION 3.31)
project(msfl_world)

set(CMAKE_CXX_STANDARD 26)

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

include_directories(lib/glad/include lib/stb_image src/wrld)

add_library(msfl_world STATIC
        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp

        # Source code
        src/wrld/World.cpp
        src/wrld/System.cpp
        src/wrld/builtins.cpp
        src/wrld/Main.cpp


        src/wrld/components/Component.cpp
        src/wrld/components/Transform.cpp
        src/wrld/components/StaticModel.cpp
        src/wrld/components/Camera.cpp
        src/wrld/components/PointLight.cpp
        src/wrld/components/DirectionalLight.cpp
        src/wrld/components/FPSControl.cpp
        src/wrld/components/Environment.cpp
        src/wrld/components/Orbiter.cpp


        src/wrld/resources/Program.cpp
        src/wrld/resources/Texture.cpp
        src/wrld/resources/Model.cpp
        src/wrld/resources/CubemapTexture.cpp
        src/wrld/resources/Material.cpp
        src/wrld/resources/WindowFramebuffer.cpp
        src/wrld/resources/Resource.cpp
        src/wrld/resources/Mesh.cpp
        src/wrld/resources/Framebuffer.cpp
        src/wrld/resources/DeferredFramebuffer.cpp
        src/wrld/resources/Rc.cpp


        src/wrld/systems/RendererSystem.cpp
        src/wrld/systems/DeferredRendererSystem.cpp


        src/wrld-gui/components.cpp
        src/wrld-gui/resources.cpp
)

target_include_directories(msfl_world PUBLIC
        "include"
        #        "lib/glfw-3.4/include"
        #        "lib/glm"
        #        "lib/assimp-6.0.2/include"
        #        "lib/imgui-1.92.2b"
        #        "lib/imgui-1.92.2b/backends"
        #        "lib/glad/include"
        #        "lib/stb_image"
)

target_link_libraries(msfl_world glfw glm assimp imgui imgui_impl_glfw imgui_impl_opengl3)

add_executable(shader_view shader_view.cpp)
target_link_libraries(shader_view msfl_world)

add_executable(proima proima.cpp)
target_link_libraries(proima msfl_world)