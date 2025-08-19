//
// Created by leo on 8/19/25.
//

#include "PointLightComponent.hpp"

namespace wrld {
    PointLightComponent::PointLightComponent(const EntityID entity_id, World &world, const glm::vec3 &color,
                                             const float intensity) :
        Component(entity_id, world), color(color), intensity(intensity) {}

    glm::vec3 PointLightComponent::get_color() const { return color; }

    void PointLightComponent::set_color(const glm::vec3 &color) { this->color = color; }

    float PointLightComponent::get_intensity() const { return intensity; }

    void PointLightComponent::set_intensity(const float intensity) { this->intensity = intensity; }
} // namespace wrld
