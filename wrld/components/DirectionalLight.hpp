//
// Created by leo on 8/23/25.
//

#ifndef DIRECTIONALLIGHT_HPP
#define DIRECTIONALLIGHT_HPP
#include "Component.hpp"
#include "glm/vec3.hpp"

namespace wrld::cpt {
    ///
    /// intensity - Value from 0 to 1.
    class DirectionalLight final : public Component {
    public:
        DirectionalLight(EntityID entity_id, World &world, const glm::vec3 &color, float intensity);

        [[nodiscard]] glm::vec3 get_color() const;
        void set_color(const glm::vec3 &color);

        [[nodiscard]] float get_intensity() const;
        void set_intensity(float intensity);

        std::string get_type() override { return "DirectionalLight"; }

    private:
        glm::vec3 color;
        float intensity;

    public:
    };

} // namespace wrld::cpt

#endif // DIRECTIONALLIGHT_HPP
