//
// Created by leo on 11/24/25.
//

#include <wrld/App.hpp>
#include <wrld/Main.hpp>
#include <wrld/builtins.hpp>
#include <wrld/components/Camera3D.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/objects/geometry/AABoundingBox.hpp>
#include <wrld/resources/Model.hpp>
#include <wrld-gui/components.hpp>
#include <wrld-gui/misc.hpp>
using namespace wrld;

class TestAABB final : public App {
public:
    void init(World &world) override {
        box = obj::AABoundingBox{glm::vec3{-1}, glm::vec3{1}};

        // Visualisation of the AABB
        const auto visu = box.as_box().get_mesh();
        const auto aabb_model = world.create_resource<rsc::Model>("aabb");
        aabb_model->from_mesh(visu, world.get_default<rsc::Material>());
        const EntityID aabb = world.create_entity("Model");
        world.attach_component<cpt::StaticModel>(aabb, aabb_model);

        // Visualisation of the ray
        const EntityID ray_entity = world.create_entity("ray_entity");
        ray_model = world.create_resource<rsc::Model>("ray_model");
        world.attach_component<cpt::StaticModel>(ray_entity, ray_model);

        // Lighting
        const EntityID sun = world.create_entity("Sun");
        world.attach_component<cpt::DirectionalLight>(sun, glm::vec3{1, 0.69, 0.35}, 0.4);
        world.attach_component<cpt::Transform>(sun);

        // Player camera
        player = builtins::flyover(world);
        world.get_component<cpt::Camera3D>(player)->set_culling(false);

        glfwSetInputMode(Main::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    void update(World &world, const double delta_time) override {
        const obj::Ray ray{ray_position, {1, 0.6, -0.3}, 100};

        ray_model->from_mesh(ray.get_mesh(), world.get_default<rsc::Material>());

        world.get_component<cpt::FPSControl>(player)->update(Main::get_window(),
                                                             delta_time);

        if (box.intersect(ray))
            Main::set_statistic("Intersect ?", "true");
        else
            Main::set_statistic("Intersect ?", "false");


        if (glfwGetKey(Main::get_window(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            Main::exit();
        }
        if (glfwGetKey(Main::get_window(), GLFW_KEY_LEFT) == GLFW_PRESS) {
            ray_position -= glm::vec3{0, 0.03, 0};
        }
        if (glfwGetKey(Main::get_window(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
            ray_position += glm::vec3{0, 0.03, 0};
        }
    }

    void ui(World &world) override { gui::render_info_window(world); }

    void exit(World &world) override {}

private:
    obj::AABoundingBox box;
    EntityID player = 0;
    Rc<rsc::Model> ray_model;

    glm::vec3 ray_position = {-2, 10, 0};
};

int main() {
    TestAABB app{};
    Main::set_window_title("Test - AABB");
    Main::run(app, 1280, 900);
    return 0;
}
