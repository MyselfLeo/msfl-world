//
// Created by leo on 9/4/25.
//

#ifndef ORBITERCONTROL_HPP
#define ORBITERCONTROL_HPP
#include <wrld/components/Component.hpp>

#include <glm/vec3.hpp>

namespace wrld::cpt {
    enum OrbiterMode { WORLD_POINT, ENTITY };

    class Orbiter final : public Component {
    public:
        Orbiter(EntityID entity_id, World &world, glm::vec3 target, float distance);
        Orbiter(EntityID entity_id, World &world, EntityID target, float distance);

        void set_target(const glm::vec3 &target);
        void set_target(EntityID target);
        void set_distance(float distance);
        void set_vert_angle(float vert_angle);
        // In degrees
        void set_hor_angle(float hor_angle);

        void set_offset(const glm::vec3 &offset);
        glm::vec3 get_offset() const;

        float get_distance() const;
        // In degrees
        float get_hor_angle() const;
        float get_vert_angle() const;
        OrbiterMode get_current_mode() const;

        /// Returns the world point currently targeted by the orbiter.
        /// It's either a fixed one (get_current_mode() returns WORLD_POINT)
        /// or the position of the targeted Entity (get_current_mode() returns ENTITY),
        /// including a potential offset.
        glm::vec3 get_target_point() const;

        /// Return the Entity targeted.
        EntityID get_target_entity() const;

        void update() const;

        std::string get_type() override { return "Orbiter"; }


    private:
        OrbiterMode mode;

        glm::vec3 target_point;
        EntityID target_entity;

        glm::vec3 offset;

        float distance;
        // In degrees
        float hor_angle;
        // In degrees
        float vert_angle;
    };

} // namespace wrld::cpt

#endif // ORBITERCONTROL_HPP
