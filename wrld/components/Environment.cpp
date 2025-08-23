//
// Created by leo on 8/22/25.
//

#include "Environment.hpp"

namespace wrld::cpt {
    AmbiantLight::AmbiantLight() : color(glm::vec4{1.0, 1.0, 1.0, 0.2}) {}

    AmbiantLight::AmbiantLight(const glm::vec4 &color) : color(color) {}

    Environment::Environment(const EntityID entity_id, World &world, const AmbiantLight ambiant_light) :
        Component(entity_id, world), ambiant_light(ambiant_light), vao(0) {
        glGenVertexArrays(1, &vao);
    }

    Environment::Environment(Environment &&other) noexcept :
        Component(other.entity_id, other.world), ambiant_light(other.ambiant_light), vao(other.vao) {
        other.vao = 0;
    }

    bool Environment::has_cubemap() const { return skybox.has_value(); }

    const AmbiantLight &Environment::get_ambiant_light() const { return ambiant_light; }

    const std::optional<std::shared_ptr<CubemapTexture>> &Environment::get_cubemap() const { return skybox; }

    void Environment::set_ambiant_light(const AmbiantLight ambiant_light) { this->ambiant_light = ambiant_light; }

    void Environment::set_cubemap(const std::shared_ptr<CubemapTexture> &cubemap) { this->skybox = cubemap; }

    void Environment::remove_cubemap() { this->skybox = std::nullopt; }

    GLuint Environment::get_vao() const { return vao; }

    Environment::~Environment() { glDeleteVertexArrays(1, &vao); }
} // namespace wrld::cpt
