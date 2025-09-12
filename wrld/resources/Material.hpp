//
// Created by leo on 9/2/25.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include "Resource.hpp"
#include "Texture.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>
#include <optional>

namespace wrld::rsc {

    class Material final : public Resource {
    public:
        Material(std::string name, World &world);

        Material(Material &other) = delete;
        Material(Material &&other) = delete;
        Material &operator=(Material &other) = delete;
        Material &operator=(Material &&other) = delete;

        void set_diffuse_map(const std::shared_ptr<Texture> &diffuse_map);
        void set_specular_map(const std::shared_ptr<Texture> &specular_map);

        void remove_diffuse_map();
        void remove_specular_map();

        [[nodiscard]] std::optional<std::shared_ptr<Texture>> get_diffuse_map() const;
        [[nodiscard]] std::optional<std::shared_ptr<Texture>> get_specular_map() const;

        void set_diffuse_color(const glm::vec4 &color);
        void set_specular_intensity(float intensity);

        [[nodiscard]] glm::vec4 get_diffuse_color() const;
        [[nodiscard]] float get_specular_intensity() const;

        void set_shininess(float shininess);

        [[nodiscard]] float get_shininess() const;

        /// If no diffuse map is specified, specify
        /// if we use Material::diffuse_color or the
        /// embedded colors in the mesh.
        void use_mesh_color(bool use);

        /// If set to False, the object won't be lit.
        /// (equivalent to a full white ambiant light).
        void do_lighting(bool do_lighting);

        [[nodiscard]] bool is_using_mesh_color() const;
        [[nodiscard]] bool is_doing_lighting() const;

    private:
        // Constant colors if maps are not specified
        glm::vec4 diffuse_color = {1.0, 1.0, 1.0, 1.0};
        float specular_intensity = 0.5;

        // Maps
        std::optional<std::shared_ptr<Texture>> diffuse_map; // Diffuse color
        std::optional<std::shared_ptr<Texture>> specular_map; // Specular intensity

        // Physical values
        float shininess = 32;

        // Config
        bool _use_mesh_color = true;
        bool _do_lighting = true;
    };
} // namespace wrld::rsc

#endif // MATERIAL_HPP
