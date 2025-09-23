//
// Created by leo on 9/12/25.
//

#include <wrld/App.hpp>
#include <wrld/Main.hpp>

#include <wrld/components/Camera.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/FPSControl.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/resources/Model.hpp>

#include <wrld-gui/components.hpp>
#include <wrld-gui/resources.hpp>

#include "assimp/postprocess.h"

#include <iostream>

using namespace wrld;

class ProIma final : public App {
public:
    ProIma() = default;

    ~ProIma() override = default;

    void init(World &world) override {
        // Load a material to be shared by every mesh.
        // In rungholt.obj, each mesh represent a block type, and each has a specific material
        // If we let the Model importer do its job, we'll have too much materials (not worth it for now).
        // We create a custom basic material with the texture attached to it, it will work just fine but
        // we'll have only 1 material meaning 1 draw call needed
        const auto texture = world.create_resource<rsc::Texture>("minecraft_texture");
        texture.get_mut()->set_texture("data/models/rungholt/house-RGBA.png", aiTextureType_DIFFUSE, false);

        const auto material = world.create_resource<rsc::Material>("city_material");
        material.get_mut()->set_diffuse_map(texture);
        material.get_mut()->set_specular_intensity(0.9);
        material.get_mut()->set_shininess(64);

        city_model = world.create_resource<rsc::Model>("city_model");
        city_model.get_mut()->from_file("data/models/rungholt/rungholt.obj", aiProcess_Triangulate | aiProcess_FlipUVs,
                                        false, material);
        // city_model.get_mut()->from_file("data/models/rungholt/house.obj", aiProcess_Triangulate | aiProcess_FlipUVs,
        //                                 false, material);

        const EntityID city_entity = world.create_entity("City");
        world.attach_component<cpt::StaticModel>(city_entity, city_model);
        world.attach_component<cpt::Transform>(city_entity);

        const EntityID camera_entity = world.create_entity("Camera");
        world.attach_component<cpt::Camera>(camera_entity, 45, Main::get_window_viewport(),
                                            world.get_default<rsc::Program>());

        world.attach_component<cpt::Transform>(camera_entity);
        control = world.attach_component<cpt::FPSControl>(camera_entity);
        const auto &env = world.attach_component<cpt::Environment>(camera_entity);
        env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 0.83, 0.64}, 0.4});
        env->set_cubemap(world.get_default<rsc::CubemapTexture>());

        const EntityID sun = world.create_entity("Sun");
        world.attach_component<cpt::DirectionalLight>(sun, glm::vec3{1, 0.69, 0.35}, 0.4);
        world.attach_component<cpt::Transform>(sun);

        for (int i = 0; i < LIGHT_COUNT; i++) {
            const EntityID light = world.create_entity(std::format("Light_{}", i));
            world.attach_component<cpt::PointLight>(
                    light, glm::vec3{(rand() % 255) / 255.0, (rand() % 255) / 255.0, (rand() % 255) / 255.0}, 10.0);
            const auto &transform = world.attach_component<cpt::Transform>(light);
            transform->set_position(
                    glm::vec3{300.0 - float(rand() % 600), float(rand() % 50), 300.0 - float(rand() % 600)});
            light_transforms[i] = transform;
        }

        glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void update(World &world, const double deltatime) override {
        this->deltatime = deltatime;

        if (glfwGetKey(Main::get_window(), GLFW_KEY_L) == GLFW_PRESS && !l_key_pressed) {
            l_key_pressed = true;
            capture_cursor = !capture_cursor;

            control->set_lock(!capture_cursor);

            if (capture_cursor) {
                glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } else if (glfwGetKey(Main::get_window(), GLFW_KEY_L) == GLFW_RELEASE) {
            l_key_pressed = false;
        }

        control->update(Main::get_window());

        // Move lights
        for (auto &trfm: light_transforms) {
            const auto &current_pos = trfm->get_position();
            auto move = glm::vec3{10 - rand() % 20, 0, 10 - rand() % 20};
            trfm->set_position(current_pos + move * static_cast<float>(deltatime));
        }
    }

    void ui(World &world) override {
        static bool show = true;
        gui::render_component_window(world, &show);
        gui::render_resources_window(world, &show);

        ImGui::Begin("Info");
        ImGui::Text("Deltatime: %.2f", deltatime);
        ImGui::Text("FPS: %d", static_cast<int>(1.0 / deltatime));
        ImGui::End();
    }

    void exit(World &world) override {}

private:
    static constexpr int LIGHT_COUNT = 100;

    Rc<rsc::Model> city_model;
    std::shared_ptr<cpt::FPSControl> control;

    std::array<std::shared_ptr<cpt::Transform>, LIGHT_COUNT> light_transforms;

    bool capture_cursor = true;
    bool l_key_pressed = false;

    double deltatime = 0.0;
};

int main() {
    ProIma app{};
    Main::set_renderer_type(DEFERRED_RENDERER);
    Main::run(app, 1280, 900);
    return 0;
}
