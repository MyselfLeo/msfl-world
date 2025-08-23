//
// Created by leo on 8/23/25.
//

#include "DirectionalLight.hpp"

namespace wrld::cpt {
    DirectionalLight::DirectionalLight(const EntityID entity_id, World &world, const glm::vec4 &color) :
        Component(entity_id, world), color(color) {}

    glm::vec4 DirectionalLight::get_color() const { return color; }

    void DirectionalLight::set_color(const glm::vec4 &color) { this->color = color; }

} // namespace wrld::cpt
