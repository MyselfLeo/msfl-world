cmake_minimum_required(VERSION 3.6)
project(msfl_world VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 26)

include(settings.cmake)

# Third party libraries
include(cmake/lib/glfw.cmake)
include(cmake/lib/glm.cmake)
include(cmake/lib/assimp.cmake)
include(cmake/lib/imgui.cmake)
include_directories(lib/glad/include lib/stb_image)

message("----- MSFL-WORLD -----")
include(cmake/configure_msflw.cmake)

set(HEADERS
        include/wrld/World.hpp
        include/wrld/System.hpp
        include/wrld/builtins.hpp
        include/wrld/Main.hpp

        include/wrld/components/Component.hpp
        include/wrld/components/Component.tpp
        include/wrld/components/Transform.hpp
        include/wrld/components/StaticModel.hpp
        include/wrld/components/Camera3D.hpp
        include/wrld/components/PointLight.hpp
        include/wrld/components/DirectionalLight.hpp
        include/wrld/components/FPSControl.hpp
        include/wrld/components/Environment3D.hpp
        include/wrld/components/Orbiter.hpp

        include/wrld/resources/Program.hpp
        include/wrld/resources/Texture.hpp
        include/wrld/resources/Model.hpp
        include/wrld/resources/CubemapTexture.hpp
        include/wrld/resources/Material.hpp
        include/wrld/resources/WindowFramebuffer.hpp
        include/wrld/resources/Resource.hpp
        include/wrld/resources/Resource.tpp
        include/wrld/resources/Framebuffer.hpp
        include/wrld/resources/DeferredFramebuffer.hpp
        include/wrld/resources/Rc.hpp
        include/wrld/resources/Rc.tpp

        include/wrld/shaders/skybox_shader.glsl.hpp
        include/wrld/shaders/deferred_second_pass.glsl.hpp
        include/wrld/shaders/compute/visibility_check.comp.hpp
        include/wrld/shaders/compute/draw_call_gen.comp.hpp
        include/wrld/shaders/vertex/forward.vert.hpp
        include/wrld/shaders/vertex/deferred.vert.hpp
        include/wrld/shaders/fragment/forward.frag.hpp
        include/wrld/shaders/fragment/deferred.frag.hpp

        include/wrld/objects/geometry/Mesh.hpp
        include/wrld/objects/geometry/MeshGroup.hpp
        include/wrld/objects/geometry/Box.hpp
        include/wrld/objects/geometry/AABoundingBox.hpp
        include/wrld/objects/geometry/BVHierarchy.hpp
        include/wrld/objects/geometry/GeoRequestable.hpp
        include/wrld/objects/geometry/Ray.hpp
        include/wrld/objects/geometry/Triangle.hpp

        include/wrld/systems/RenderSystem.hpp
        include/wrld/systems/ForwardRenderSystem.hpp
        include/wrld/systems/DeferredRenderSystem.hpp

        include/wrld/tools/ModelTool.hpp
        include/wrld/tools/Geometry.hpp

        include/wrld/macros.hpp
        include/wrld/Singleton.hpp

        include/wrld-gui/components.hpp
        include/wrld-gui/resources.hpp
        include/wrld-gui/misc.hpp
)

set(SOURCE
        src/wrld/World.cpp
        src/wrld/System.cpp
        src/wrld/builtins.cpp
        src/wrld/Main.cpp

        src/wrld/components/Component.cpp
        src/wrld/components/Transform.cpp
        src/wrld/components/StaticModel.cpp
        src/wrld/components/Camera3D.cpp
        src/wrld/components/PointLight.cpp
        src/wrld/components/DirectionalLight.cpp
        src/wrld/components/FPSControl.cpp
        src/wrld/components/Environment3D.cpp
        src/wrld/components/Orbiter.cpp

        src/wrld/resources/Program.cpp
        src/wrld/resources/Texture.cpp
        src/wrld/resources/Model.cpp
        src/wrld/resources/CubemapTexture.cpp
        src/wrld/resources/Material.cpp
        src/wrld/resources/WindowFramebuffer.cpp
        src/wrld/resources/Resource.cpp
        src/wrld/resources/Framebuffer.cpp
        src/wrld/resources/DeferredFramebuffer.cpp
        src/wrld/resources/Rc.cpp

        src/wrld/objects/geometry/Mesh.cpp
        src/wrld/objects/geometry/MeshGroup.cpp
        src/wrld/objects/geometry/Box.cpp
        src/wrld/objects/geometry/AABoundingBox.cpp
        src/wrld/objects/geometry/Ray.cpp
        src/wrld/objects/geometry/BVHierarchy.cpp
        src/wrld/objects/geometry/GeoRequestable.cpp
        src/wrld/objects/geometry/Triangle.cpp

        src/wrld/systems/RenderSystem.cpp
        src/wrld/systems/ForwardRenderSystem.cpp
        src/wrld/systems/DeferredRenderSystem.cpp

        src/wrld/tools/ModelTool.cpp
        src/wrld/tools/Geometry.cpp

        src/wrld-gui/components.cpp
        src/wrld-gui/resources.cpp
        src/wrld-gui/misc.cpp
)

add_library(msfl_world STATIC
        # Libraries that are not compiled beforehand
        lib/glad/src/glad.c
        lib/stb_image/stb_image.cpp
        ${HEADERS}
        ${SOURCE}
)

target_include_directories(msfl_world PUBLIC
        lib/glfw/include
        lib/glm
        lib/assimp/include
        lib/imgui
        lib/imgui/backends
        lib/glad/include
        lib/stb_image
        include
)


target_link_libraries(msfl_world PUBLIC glfw glm assimp imgui imgui_impl_glfw imgui_impl_opengl3)

if (MSFLW_USE_OPENMP)
    target_link_libraries(msfl_world PUBLIC OpenMP::OpenMP_CXX)
endif ()

target_compile_definitions(msfl_world PRIVATE MSFL_WORLD_VERSION="${msfl_world_VERSION}")
