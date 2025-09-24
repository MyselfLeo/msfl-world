//
// Created by leo on 8/22/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include <wrld/resources/CubemapTexture.hpp>

#include <optional>

namespace wrld::cpt {
    /// intensity - Value from 0 to 1.
    struct AmbiantLight {
        AmbiantLight();
        AmbiantLight(const glm::vec3 &color, float intensity);

        glm::vec3 color;
        float intensity;
    };

    /// Represent a rendering context for an environment.
    /// Specifies ambiant light, cubemap, etc.
    /// Must be attached to an Entity with a Camera component.
    class Environment final : public Component {
    public:
        Environment(EntityID entity_id, World &world, AmbiantLight ambiant_light = {});

        Environment(EntityID entity_id, World &world, const Rc<rsc::CubemapTexture> &cubemap,
                    AmbiantLight ambiant_light = {});

        Environment(Environment &&other) noexcept;

        Environment &operator=(Environment &&other) noexcept = delete;

        // [[nodiscard]] bool has_cubemap() const;

        [[nodiscard]] const AmbiantLight &get_ambiant_light() const;
        [[nodiscard]] std::optional<Rc<rsc::CubemapTexture>> get_cubemap() const;

        void set_ambiant_light(AmbiantLight ambiant_light);
        void set_cubemap(const Rc<rsc::CubemapTexture> &cubemap);
        void remove_cubemap();

        [[nodiscard]] GLuint get_vao() const;

        ~Environment() override;

        std::string get_type() override { return "Environment"; }

    private:
        GLuint vao; // Required even with no data inside
        AmbiantLight ambiant_light;
    };

} // namespace wrld::cpt
