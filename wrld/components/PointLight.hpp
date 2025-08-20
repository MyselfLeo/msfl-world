//
// Created by leo on 8/19/25.
//

#ifndef POINTLIGHTCOMPONENT_HPP
#define POINTLIGHTCOMPONENT_HPP
#include "Component.hpp"

#include <glm/vec3.hpp>

namespace wrld::cpt {

    class PointLight final : public Component {
    public:
        PointLight(EntityID entity_id, World &world, const glm::vec3 &color, float intensity);

        [[nodiscard]] glm::vec3 get_color() const;
        void set_color(const glm::vec3 &color);
        [[nodiscard]] float get_intensity() const;
        void set_intensity(float intensity);


    private:
        glm::vec3 color;
        float intensity;
    };

} // namespace wrld::cpt

#endif // POINTLIGHTCOMPONENT_HPP
