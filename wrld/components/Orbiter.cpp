//
// Created by leo on 9/4/25.
//

#include "Orbiter.hpp"

#include "Transform.hpp"
#include "World.hpp"

#include <iostream>

namespace wrld::cpt {
    Orbiter::Orbiter(const EntityID entity_id, World &world, const glm::vec3 target, const float distance) :
        Component(entity_id, world), mode(WORLD_POINT), target_point(std::move(target)), target_entity(0),
        distance(distance), hor_angle(0), vert_angle(45) {}

    Orbiter::Orbiter(const EntityID entity_id, World &world, const EntityID target, const float distance) :
        Component(entity_id, world), mode(ENTITY), target_point({0, 0, 0}), target_entity(target), distance(distance),
        hor_angle(0), vert_angle(45) {}

    void Orbiter::set_target(const glm::vec3 &target) {
        target_point = target;
        mode = WORLD_POINT;
    }

    void Orbiter::set_target(const EntityID target) {
        target_entity = target;
        mode = ENTITY;
    }

    void Orbiter::set_distance(const float distance) {
        this->distance = distance;
        if (this->distance < 0)
            this->distance = 0;
    }

    void Orbiter::set_vert_angle(const float vert_angle) {
        this->vert_angle = vert_angle;
        if (this->vert_angle > 89)
            this->vert_angle = 89;
        else if (this->vert_angle < -89)
            this->vert_angle = -89;
    }

    void Orbiter::set_hor_angle(const float hor_angle) { this->hor_angle = hor_angle; }

    float Orbiter::get_hor_angle() const { return hor_angle; }

    float Orbiter::get_vert_angle() const { return vert_angle; }

    float Orbiter::get_distance() const { return distance; }

    OrbiterMode Orbiter::get_current_mode() const { return mode; }

    glm::vec3 Orbiter::get_target_point() const {
        switch (mode) {
            case WORLD_POINT:
                return target_point;
            case ENTITY: {
                if (const auto transform_opt = world.get_component_opt<Transform>(target_entity);
                    transform_opt.has_value()) {
                    return transform_opt.value()->get_position();
                }
                return {0, 0, 0};
            }
            default:
                assert(false);
        }
    }

    void Orbiter::update() const {
        // Get the entity's transform
        const auto transform_opt = world.get_component_opt<Transform>(entity_id);
        if (!transform_opt.has_value())
            return;

        const auto transform = transform_opt.value();

        const float hor_angle_rad = glm::radians(hor_angle);
        const float vert_angle_rad = glm::radians(vert_angle);

        // Compute new transform
        const glm::vec3 vector = glm::normalize(glm::vec3{cos(hor_angle_rad) * cos(vert_angle_rad), sin(vert_angle_rad),
                                                          sin(hor_angle_rad) * cos(vert_angle_rad)});
        const glm::vec3 position = get_target_point() + vector * distance;

        transform->set_position(position);
        transform->look_at(get_target_point(), {0, 1, 0});
    }
} // namespace wrld::cpt
