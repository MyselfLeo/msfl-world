//
// Created by leo on 8/19/25.
//

#include "PointLight.hpp"

namespace wrld::cpt {
    PointLight::PointLight(const EntityID entity_id, World &world, const glm::vec3 &color, const float intensity) :
        Component(entity_id, world), color(color), intensity(intensity) {}

    glm::vec3 PointLight::get_color() const { return color; }

    void PointLight::set_color(const glm::vec3 &color) { this->color = color; }

    float PointLight::get_intensity() const { return intensity; }

    void PointLight::set_intensity(const float intensity) { this->intensity = intensity; }
} // namespace wrld::cpt
