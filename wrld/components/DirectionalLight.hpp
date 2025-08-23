//
// Created by leo on 8/23/25.
//

#ifndef DIRECTIONALLIGHT_HPP
#define DIRECTIONALLIGHT_HPP
#include "Component.hpp"
#include "glm/vec4.hpp"

namespace wrld::cpt {

    class DirectionalLight final : public Component {
    public:
        DirectionalLight(EntityID entity_id, World &world, const glm::vec4 &color);

        [[nodiscard]] glm::vec4 get_color() const;
        void set_color(const glm::vec4 &color);

    private:
        glm::vec4 color;
    };

} // namespace wrld::cpt

#endif // DIRECTIONALLIGHT_HPP
