//
// Created by leo on 8/11/25.
//

#ifndef TRANSFORMCOMPONENT_HPP
#define TRANSFORMCOMPONENT_HPP
#include <wrld/components/Component.hpp>
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace wrld::cpt {

    /// Gives an Entity a position, a rotation and a scale.
    class Transform final : public Component {
    public:
        // The constructor cannot take other parameters than that
        Transform(EntityID entity_id, World &world, const glm::vec3 &position = glm::vec3(0.0f),
                  const glm::quat &rotation = glm::quat(1, 0, 0, 0), const glm::vec3 &scale = glm::vec3(1.0f));

        [[nodiscard]] glm::vec3 get_position() const;
        [[nodiscard]] glm::quat get_rotation() const;
        [[nodiscard]] glm::vec3 get_scale() const;

        /// Return the direction of the Transform as a vec3
        [[nodiscard]] glm::vec3 get_direction() const;

        void set_position(const glm::vec3 &position);
        void set_rotation(const glm::quat &rotation);
        void set_scale(const glm::vec3 &scale);

        /// Look at a specific point in world space.
        void look_at(const glm::vec3 &target, const glm::vec3 &up);

        /// Look in a specific direction in world space.
        /// Equivalent to look_at(position + direction, up).
        void look_towards(const glm::vec3 &direction, const glm::vec3 &up);

        [[nodiscard]] glm::mat4x4 model_matrix() const;
        [[nodiscard]] glm::mat4x4 translate_matrix() const;
        [[nodiscard]] glm::mat4x4 rotation_matrix() const;
        [[nodiscard]] glm::mat4x4 scale_matrix() const;

        std::string get_type() override { return "Transform"; }

    private:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

} // namespace wrld::cpt

#endif // TRANSFORMCOMPONENT_HPP
