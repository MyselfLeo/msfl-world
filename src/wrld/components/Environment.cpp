//
// Created by leo on 8/22/25.
//

#include <wrld/components/Environment.hpp>

namespace wrld::cpt {
    AmbiantLight::AmbiantLight() : color(glm::vec3{1.0, 1.0, 1.0}), intensity(0.2) {}

    AmbiantLight::AmbiantLight(const glm::vec3 &color, const float intensity) : color(color), intensity(intensity) {}

    Environment::Environment(const EntityID entity_id, World &world, const AmbiantLight ambiant_light) :
        Component(entity_id, world), vao(0), ambiant_light(ambiant_light) {
        glGenVertexArrays(1, &vao);
    }

    Environment::Environment(const EntityID entity_id, World &world, const Rc<rsc::CubemapTexture> &cubemap,
                             const AmbiantLight ambiant_light) :
        Component(entity_id, world), vao(0), ambiant_light(ambiant_light) {
        attach_resource("skybox", cubemap);
        glGenVertexArrays(1, &vao);
    }

    Environment::Environment(Environment &&other) noexcept :
        Component(other.entity_id, other.world), ambiant_light(other.ambiant_light), vao(other.vao) {
        other.vao = 0;
    }

    const AmbiantLight &Environment::get_ambiant_light() const { return ambiant_light; }

    std::optional<Rc<rsc::CubemapTexture>> Environment::get_cubemap() const {
        if (!has_resource("skybox"))
            return std::nullopt;
        return get_resource<rsc::CubemapTexture>("skybox");
    }

    void Environment::set_ambiant_light(const AmbiantLight ambiant_light) { this->ambiant_light = ambiant_light; }

    void Environment::set_cubemap(const Rc<rsc::CubemapTexture> &cubemap) { attach_resource("skybox", cubemap); }

    void Environment::remove_cubemap() { detach_resource("skybox"); }

    GLuint Environment::get_vao() const { return vao; }

    Environment::~Environment() { glDeleteVertexArrays(1, &vao); }
} // namespace wrld::cpt
