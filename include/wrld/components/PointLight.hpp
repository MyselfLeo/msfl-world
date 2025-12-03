//
// Created by leo on 8/19/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include <wrld/components/Transform.hpp>

#include <glm/vec3.hpp>

namespace wrld::cpt {

    /// intensity - Value from 0 to +inf. The bigger, the more energy the light emits.
    class PointLight final : public Component {
    public:
        using required_components = std::tuple<Transform>;

        PointLight(EntityID entity_id, World &world, const glm::vec3 &color, float intensity);

        [[nodiscard]] glm::vec3 get_color() const;
        void set_color(const glm::vec3 &color);
        [[nodiscard]] float get_intensity() const;
        void set_intensity(float intensity);

        static std::string get_type() { return "PointLight"; }

    private:
        glm::vec3 color;
        float intensity;
    };

} // namespace wrld::cpt
