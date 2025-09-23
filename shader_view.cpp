//
// Created by leo on 9/11/25.
//

#include <wrld/App.hpp>
#include <wrld/Main.hpp>
#include <wrld/builtins.hpp>
#include <wrld/logs.hpp>
#include <wrld/components/Camera.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/Orbiter.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/resources/Model.hpp>

#include <wrld-gui/components.hpp>
#include <wrld-gui/resources.hpp>

#include <wrld/shaders/vertex/default_shader.hpp>
#include <wrld/shaders/fragment/default_shader.hpp>

#include "imgui.h"
#include "assimp/postprocess.h"

#include <iostream>

using namespace wrld;

class BlobApp final : public App {
public:
    explicit BlobApp(const std::string &model_path) : model_path(model_path) {}

    ~BlobApp() override {}

    void init(World &world) override {
        shader = world.create_resource<rsc::Program>("shader");
        shader.get_mut()->from_source(shader::DEFAULT_VERTEX, shader::DEFAULT_FRAGMENT);

        model = world.create_resource<rsc::Model>("user_model");
        model.get_mut()->from_file(model_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals, false);

        const EntityID model_entity = world.create_entity("Model");
        world.attach_component<cpt::StaticModel>(model_entity, model);
        model_transform = world.attach_component<cpt::Transform>(model_entity);

        builtins::create_grid(world);
        builtins::create_axis(world);

        const EntityID camera_entity = world.create_entity("Camera");
        camera = world.attach_component<cpt::Camera>(camera_entity, 45, Main::get_window_viewport(), shader);
        world.attach_component<cpt::Transform>(camera_entity);
        orbiter = world.attach_component<cpt::Orbiter>(camera_entity, model_entity, 2);
        orbiter->set_offset({0, 0, 0});
        world.attach_component<cpt::Environment>(camera_entity)
                ->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 1.0, 1.0}, 0.0});

        const EntityID dir_light = world.create_entity("Light");
        world.attach_component<cpt::DirectionalLight>(dir_light, glm::vec3{1.0, 1.0, 1.0}, 1.0);
        world.attach_component<cpt::Transform>(dir_light)->look_towards({-1, 0, -1}, {0, 1, 0});

        // bool show_demo_window = true;
        shade_reloading = false;
    }

    void update(World &world, const double deltatime) override {
        current_deltatime = deltatime;

        // Shader reloading
        {
            if (glfwGetKey(Main::get_window(), GLFW_KEY_R) == GLFW_PRESS && !shade_reloading) {
                shade_reloading = true;
                camera->get_program().get_mut()->reload();
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_R) == GLFW_RELEASE) {
                shade_reloading = false;
            }
        }

        // Rotate the model
        {
            const auto rotation = glm::quat(glm::vec3{0, glm::radians(rotation_rate) * deltatime, 0});
            const auto curr_rotation = model_transform->get_rotation();
            model_transform->set_rotation(rotation * curr_rotation);
        }

        // Orbiter control
        {
            if (glfwGetKey(Main::get_window(), GLFW_KEY_UP) == GLFW_PRESS) {
                orbiter->set_vert_angle(orbiter->get_vert_angle() + camera_speed);
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_DOWN) == GLFW_PRESS) {
                orbiter->set_vert_angle(orbiter->get_vert_angle() - camera_speed);
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
                orbiter->set_hor_angle(orbiter->get_hor_angle() + camera_speed);
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
                orbiter->set_hor_angle(orbiter->get_hor_angle() - camera_speed);
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
                orbiter->set_distance(orbiter->get_distance() * 0.95);
            }
            if (glfwGetKey(Main::get_window(), GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
                orbiter->set_distance(orbiter->get_distance() / 0.95);
            }
            orbiter->update();
        }
    }

    void ui(World &world) override {
        static bool show_demo = false;
        static bool show_components = false;
        static bool show_resources = false;

        ImGui::Begin("Info");
        ImGui::Text("%s", std::format("DT: {}ms", rotation_rate).c_str());
        if (ImGui::Button("Show/Hide demo"))
            show_demo = !show_demo;
        if (ImGui::Button("Show/Hide components"))
            show_components = !show_components;
        if (ImGui::Button("Show/Hide resources"))
            show_resources = !show_resources;
        ImGui::End();

        if (show_demo)
            ImGui::ShowDemoWindow(&show_demo);
        if (show_components)
            gui::render_component_window(world, &show_components);
        if (show_resources)
            gui::render_resources_window(world, &show_resources);
    }

    void exit(World &world) override { std::cout << "Goodbye!" << std::endl; }

private:
    // Input from user
    std::string model_path;

    // Data
    Rc<rsc::Model> model;
    Rc<rsc::Program> shader;

    std::shared_ptr<cpt::Transform> model_transform;
    std::shared_ptr<cpt::Camera> camera;
    std::shared_ptr<cpt::Orbiter> orbiter;

    // Runtime variables
    bool shade_reloading = false;
    double current_deltatime = 0;
    float rotation_rate = 30.0; // Angle per second
    float camera_speed = 1.5;
};

int main() {
    BlobApp app("data/models/queen/queen.off");

    wrldInfo("Start");
    Main::run(app, 1280, 900);
    return 0;
}
