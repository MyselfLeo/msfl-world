#include "builtins.hpp"
#include "components/PointLight.hpp"


#include <format>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>


#include "wrld/Logs.hpp"
#include "wrld/resources/Program.hpp"
#include "wrld/World.hpp"
#include "wrld/components/Camera.hpp"
#include "wrld/components/DirectionalLight.hpp"
#include "wrld/components/FPSControl.hpp"
#include "wrld/components/StaticModel.hpp"
#include "wrld/components/Transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "resources/WindowFramebuffer.hpp"
#include "wrld/systems/RendererSystem.hpp"

using namespace wrld;

// static bool capture_cursor;
// std::shared_ptr<cpt::FPSControl> fps_control;
// static std::shared_ptr<rsc::WindowViewport> window_viewport;

// void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
//     if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
//         if (capture_cursor) {
//             wrldInfo("Freeing cursor");
//             capture_cursor = false;
//             fps_control->set_lock(true);
//         } else {
//             wrldInfo("Capturing cursor");
//             glfwSetCursorPos(window, 0, 0);
//             capture_cursor = true;
//             fps_control->set_lock(false);
//         }
//     }
// }


int main() {
    GLFWwindow *window = init_gl(800, 600);

    capture_cursor = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    World world;

    window_viewport = std::make_shared<rsc::WindowFramebuffer>(window);
    glfwSetWindowSizeCallback(window, window_resize_callback);

    wrldInfo("Initialising systems");
    RendererSystem renderer{world, window};

    wrldInfo("Loading model");
    const auto &backpack_model =
            world.create_resource<rsc::Model>("backpack_model", "data/models/backpack/backpack.obj");
    const auto &myshape_model = world.create_resource<rsc::Model>("myshape_model", "data/models/myshape/myshape.obj");
    const auto &cube_model = world.create_resource<rsc::Model>("cube_model", "data/models/cube/cube.obj");

    wrldInfo("Loading skybox");
    auto skybox = std::make_shared<rsc::CubemapTexture>(std::vector<std::string>{
            "data/textures/lake_cm/right.jpg", "data/textures/lake_cm/left.jpg", "data/textures/lake_cm/top.jpg",
            "data/textures/lake_cm/bottom.jpg", "data/textures/lake_cm/front.jpg", "data/textures/lake_cm/back.jpg"});

    wrldInfo("Creating entities");
    const EntityID backpack = world.create_entity();
    world.attach_component<cpt::StaticModel>(backpack, backpack_model);
    world.attach_component<cpt::Transform>(backpack, glm::vec3{4.0, 0.0, 0.0});

    const EntityID myshape = world.create_entity();
    world.attach_component<cpt::StaticModel>(myshape, myshape_model);
    world.attach_component<cpt::Transform>(myshape, glm::vec3{-4.0, 0.0, 0.0});

    builtins::create_grid(world, 100);
    builtins::create_axis(world);

    const EntityID camera_entity = world.create_entity();
    auto camera = world.attach_component<cpt::Camera>(camera_entity, 45, window_viewport);
    auto camera_transform = world.attach_component<cpt::Transform>(camera_entity, glm::vec3{0.0, 0.0, 8.0});
    fps_control = world.attach_component<cpt::FPSControl>(camera_entity);
    auto env = world.attach_component<cpt::Environment>(camera_entity);
    env->set_cubemap(skybox);
    env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 1.0, 1.0}, 0.4});

    const EntityID dir_light = world.create_entity();
    world.attach_component<cpt::DirectionalLight>(dir_light, glm::vec3{1.0, 1.0, 1.0}, 0.4);
    const auto dir_light_transform = world.attach_component<cpt::Transform>(dir_light);
    world.attach_component<cpt::StaticModel>(dir_light, cube_model);

    dir_light_transform->set_scale(glm::vec3{0.1, 0.1, 0.1});
    dir_light_transform->set_position(glm::vec3{0, 1, 0});
    dir_light_transform->look_towards(glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0});

    /*const EntityID light1 = world.create_entity();
    world.attach_component<cpt::PointLight>(light1, glm::vec3{0.0, 1.0, 0.0}, 30.0);
    const auto light_transform1 = world.attach_component<cpt::Transform>(light1);
    world.attach_component<cpt::StaticModel>(light1, cube_model);
    light_transform1->set_scale(glm::vec3{0.1, 0.1, 0.1});
    const EntityID light2 = world.create_entity();
    world.attach_component<cpt::PointLight>(light2, glm::vec3{1.0, 0.0, 0.0}, 30.0);
    const auto light_transform2 = world.attach_component<cpt::Transform>(light2);
    world.attach_component<cpt::StaticModel>(light2, cube_model);
    light_transform2->set_scale(glm::vec3{0.1, 0.1, 0.1});*/

    const EntityID light3 = world.create_entity();
    world.attach_component<cpt::PointLight>(light3, glm::vec3{0.0, 0.0, 1.0}, 5.0);
    const auto light_transform3 = world.attach_component<cpt::Transform>(light3);
    world.attach_component<cpt::StaticModel>(light3, cube_model);
    light_transform3->set_scale(glm::vec3{0.1, 0.1, 0.1});

    bool show_demo_window = true;
    bool reloading = false;

    while (!glfwWindowShouldClose(window)) {
        if (capture_cursor) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !reloading) {
            reloading = true;
            camera->get_program()->reload();
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            reloading = false;
        }
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        const double time = glfwGetTime();

        // camera_transform->set_towards({0, 0, 0});
        fps_control->update(window);

        // Rotate backpack
        const auto ROTATION_RATE = glm::quat(glm::vec3{0, 0.005, 0});

        const auto backpack_transform = world.get_component<cpt::Transform>(backpack);
        auto curr_rotation = backpack_transform->get_rotation();
        backpack_transform->set_rotation(ROTATION_RATE * curr_rotation);

        const auto myshape_transform = world.get_component<cpt::Transform>(myshape);
        curr_rotation = myshape_transform->get_rotation();
        myshape_transform->set_rotation(-ROTATION_RATE * curr_rotation);

        curr_rotation = dir_light_transform->get_rotation();
        dir_light_transform->set_rotation(ROTATION_RATE * curr_rotation);

        // light_transform1->set_position({0, sin(time) * 4, 0});
        // light_transform2->set_position({0, sin(time + M_PI) * 4, 0});
        light_transform3->set_position({0, sin(time + M_PI / 2) * 4, 0});

        renderer.exec();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!capture_cursor) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
