//
// Created by leo on 8/23/25.
//

#include "DirectionalLight.hpp"

namespace wrld::cpt {
    DirectionalLight::DirectionalLight(const EntityID entity_id, World &world, const glm::vec3 &color,
                                       const float intensity) :
        Component(entity_id, world), color(color), intensity(intensity) {}

    glm::vec3 DirectionalLight::get_color() const { return color; }

    void DirectionalLight::set_color(const glm::vec3 &color) { this->color = color; }

    float DirectionalLight::get_intensity() const { return intensity; }

    void DirectionalLight::set_intensity(const float intensity) { this->intensity = intensity; }

} // namespace wrld::cpt
