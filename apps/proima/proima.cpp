//
// Created by leo on 9/12/25.
//

#include <wrld/App.hpp>
#include <wrld/Main.hpp>

#include <wrld/components/Camera3D.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/FPSControl.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/resources/Model.hpp>

#include <wrld-gui/components.hpp>
#include <wrld-gui/misc.hpp>

#include "assimp/postprocess.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <wrld/builtins.hpp>
#include <wrld/logs.hpp>
#include <wrld/tools/ModelTool.hpp>

using namespace wrld;

class ProIma final : public App {
public:
    ProIma() = default;

    ~ProIma() override = default;

    void init(World &world) override {
        // Load a material to be shared by every mesh.
        // In rungholt.obj, each mesh represent a block type, and each has a specific
        // material If we let the Model importer do its job, we'll have too much materials
        // (not worth it for now). We create a custom basic material with the texture
        // attached to it, it will work just fine but we'll have only 1 material meaning 1
        // draw call needed
        const auto texture = world.create_resource<rsc::Texture>("minecraft_texture");
        texture.get_mut()->from_file("apps/proima/rungholt/house-RGBA.png",
                                     aiTextureType_DIFFUSE, false);

        const auto material = world.create_resource<rsc::Material>("city_material");
        material->set_diffuse_map(texture);
        material->set_specular_intensity(0.9);
        material->set_shininess(64);

        auto city_model = world.create_resource<rsc::Model>("city_model");
        city_model->from_file("apps/proima/rungholt/rungholt.obj",
                              aiProcess_Triangulate | aiProcess_FlipUVs, false, material);

        ray_model = world.create_resource<rsc::Model>("ray_model");
        const EntityID ray_entity = world.create_entity("ray entity");
        world.attach_component<cpt::StaticModel>(ray_entity, ray_model);

        wrldInfo("Computing BVH");
        rungholt_bvh = city_model->compute_bvh({}, 20);

        wrldInfo("Splitting model");
        const auto &split_models = tools::ModelTool::split_in_grid(world, city_model, 50);
        world.destroy_resource<rsc::Model>(city_model);

        // Create an entity for each split models
        for (int i = 0; i < split_models.size(); i++) {
            auto &s = split_models[i];
            const EntityID city_crumb = world.create_entity("city_crumb");
            world.attach_component<cpt::StaticModel>(city_crumb, s);
            world.attach_component<cpt::Transform>(city_crumb);
        }

        const EntityID camera_entity = world.create_entity("Camera");
        camera_3D = world.attach_component<cpt::Camera3D>(
                camera_entity, 45, true, Main::get_window_viewport(),
                world.get_default<rsc::Program>());

        camera_transform = world.attach_component<cpt::Transform>(camera_entity);
        camera_transform->set_position(glm::vec3{270, 13, -140});
        camera_transform->set_rotation(glm::quat{glm::vec3{0, 90, 0}});


        control = world.attach_component<cpt::FPSControl>(camera_entity);
        control->enable_keyboard_control(false);

        const auto &env = world.attach_component<cpt::Environment>(camera_entity);
        env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 0.83, 0.64}, 0.3});
        env->set_cubemap(world.get_default<rsc::CubemapTexture>());

        const EntityID sun = world.create_entity("Sun");
        world.attach_component<cpt::DirectionalLight>(sun, glm::vec3{1, 0.69, 0.35}, 0.4);
        const auto sun_transform = world.attach_component<cpt::Transform>(sun);
        sun_transform->set_rotation(glm::quat{glm::vec3{200, 0, 0}});

        for (int i = 0; i < LIGHT_COUNT; i++) {
            const EntityID light = world.create_entity(std::format("Light_{}", i));
            world.attach_component<cpt::PointLight>(light,
                                                    glm::vec3{(rand() % 255) / 255.0,
                                                              (rand() % 255) / 255.0,
                                                              (rand() % 255) / 255.0},
                                                    10.0);
            const auto &transform = world.attach_component<cpt::Transform>(light);
            transform->set_position(glm::vec3{300.0 - float(rand() % 600),
                                              float(rand() % 40) + 10,
                                              300.0 - float(rand() % 600)});
            light_transforms[i] = transform;
        }

        glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void update(World &world, const double delta_time) override {
        constexpr glm::vec3 down{0, -1, 0};

        this->deltatime = delta_time;

        if (glfwGetKey(Main::get_window(), GLFW_KEY_L) == GLFW_PRESS && !l_key_pressed) {
            l_key_pressed = true;
            capture_cursor = !capture_cursor;

            control->enable_mouse_control(capture_cursor);

            if (capture_cursor) {
                glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            } else {
                glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        } else if (glfwGetKey(Main::get_window(), GLFW_KEY_L) == GLFW_RELEASE) {
            l_key_pressed = false;
        }

        control->update(Main::get_window(), delta_time);

        // Move lights
        for (auto &trfm: light_transforms) {
            const auto &current_pos = trfm->get_position();
            auto move = glm::vec3{10 - rand() % 20, 0, 10 - rand() % 20};
            trfm->set_position(current_pos + move * static_cast<float>(deltatime));
        }

        // Jump
        if (glfwGetKey(Main::get_window(), GLFW_KEY_SPACE) == GLFW_PRESS && !jumping) {
            jumping = true;
            v_speed += JUMP_FORCE;
        }

        // Gravity
        const glm::vec3 camera_pos = camera_transform->get_position();
        const obj::Ray ray{camera_pos, down, 100};
        const auto my_height = rungholt_bvh.intersect(ray);

        if (delta_time < 0.1)
            v_speed += GRAVITY * delta_time;

        if (my_height.has_value()) {
            if (my_height.value() <= PLAYER_HEIGHT && v_speed < 0.0) {
                v_speed = 0.0;
                const float diff = PLAYER_HEIGHT - my_height.value();
                camera_transform->set_position(camera_transform->get_position() +
                                               glm::vec3{0, diff, 0});
                jumping = false;
            }
        }

        camera_transform->set_position(camera_transform->get_position() +
                                       glm::vec3{0, v_speed, 0});

        // Walk
        glm::vec3 hor_walk{0};
        glm::vec3 dir_forward = camera_transform->forward();
        dir_forward.y = 0;
        dir_forward = glm::normalize(dir_forward);
        glm::vec3 dir_right = camera_transform->right();
        dir_right.y = 0;
        dir_right = glm::normalize(dir_right);

        if (glfwGetKey(Main::get_window(), GLFW_KEY_W) == GLFW_PRESS) {
            hor_walk += dir_forward;
        }
        if (glfwGetKey(Main::get_window(), GLFW_KEY_S) == GLFW_PRESS) {
            hor_walk -= dir_forward;
        }
        if (glfwGetKey(Main::get_window(), GLFW_KEY_A) == GLFW_PRESS) {
            hor_walk -= dir_right;
        }
        if (glfwGetKey(Main::get_window(), GLFW_KEY_D) == GLFW_PRESS) {
            hor_walk += dir_right;
        }

        if (glm::length(hor_walk) > 0)
            hor_walk = glm::normalize(hor_walk) * PLAYER_SPEED *
                       static_cast<float>(delta_time);
        if (glfwGetKey(Main::get_window(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            hor_walk *= 2;

        // Test if we can walk (i.e the place in front is not too high)
        // How do we test that : we throw a Ray at the Y level where it is no longer
        // possible to walk
        // If it touches before the radius of the player then we are against the wall
        // and cannot walk without jumping
        if (hor_walk.x != 0 || hor_walk.z != 0) {
            glm::vec3 collision_ray_origin = camera_transform->get_position();
            collision_ray_origin.y -= 0.2;

            obj::Ray collision_ray{collision_ray_origin, hor_walk, 5.0};
            const auto collision = rungholt_bvh.intersect(collision_ray);

            if (!collision.has_value() || collision.value() > PLAYER_RADIUS) {
                // We can walk safely
                camera_transform->set_position(camera_transform->get_position() +
                                               hor_walk);
            }
        }
    }

    void ui(World &world) override {
        // gui::render_component_window(world);
        gui::render_info_window(world);
    }

    void exit(World &world) override {}

private:
    static constexpr float PLAYER_RADIUS = 0.3;
    static constexpr double JUMP_FORCE = 0.4;
    static constexpr double WALK_THRESHOLD = 2.0;
    static constexpr float PLAYER_SPEED = 10.0;
    static constexpr float PLAYER_HEIGHT = 2.0;
    static constexpr float GRAVITY = -0.98;
    static constexpr int LIGHT_COUNT = 100;

    std::shared_ptr<cpt::FPSControl> control;

    std::shared_ptr<cpt::Transform> camera_transform;
    std::shared_ptr<cpt::Camera3D> camera_3D;

    std::array<std::shared_ptr<cpt::Transform>, LIGHT_COUNT> light_transforms;

    obj::BVHierarchy<obj::Triangle> rungholt_bvh;

    Rc<rsc::Model> ray_model;

    bool capture_cursor = true;
    bool l_key_pressed = false;
    bool jumping = false;

    float v_speed = 0.0;

    double deltatime = 0.0;
};

int main() {
    ProIma app{};
    Main::set_window_title("TP PROIMA");
    Main::set_renderer_type(RendererType::DeferredRenderer);
    Main::run(app, 1280, 900);
    return 0;
}
