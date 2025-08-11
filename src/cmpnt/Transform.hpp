//
// Created by leo on 8/11/25.
//

#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP
#include "Component.hpp"
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace wrld {

    class Transform final : public Component {
    public:
        // The constructor cannot take other parameters than that
        Transform(EntityID entity_id, World &world);

        [[nodiscard]] glm::vec3 get_position() const;
        [[nodiscard]] glm::quat get_rotation() const;
        [[nodiscard]] glm::vec3 get_scale() const;

        void set_position(const glm::vec3 &position);
        void set_rotation(const glm::quat &rotation);
        void set_scale(const glm::vec3 &scale);

        glm::mat4x4 model_matrix() const;

    private:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

} // namespace wrld

#endif // TRANSFORM_HPP
