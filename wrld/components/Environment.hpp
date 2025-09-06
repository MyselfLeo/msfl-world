//
// Created by leo on 8/22/25.
//

#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP
#include "Component.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "resources/CubemapTexture.hpp"

#include <memory>
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

        Environment(Environment &&other) noexcept;

        Environment &operator=(Environment &&other) noexcept = delete;

        [[nodiscard]] bool has_cubemap() const;

        [[nodiscard]] const AmbiantLight &get_ambiant_light() const;
        [[nodiscard]] const std::optional<std::shared_ptr<rsc::CubemapTexture>> &get_cubemap() const;

        void set_ambiant_light(AmbiantLight ambiant_light);
        void set_cubemap(const std::shared_ptr<rsc::CubemapTexture> &cubemap);
        void remove_cubemap();

        GLuint get_vao() const;

        ~Environment() override;

    private:
        GLuint vao; // Required even with no data inside
        AmbiantLight ambiant_light;
        std::optional<std::shared_ptr<rsc::CubemapTexture>> skybox;
    };

} // namespace wrld::cpt

#endif // ENVIRONMENT_HPP
