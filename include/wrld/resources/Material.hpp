//
// Created by leo on 9/2/25.
//

#pragma once

#include <wrld/resources/Resource.hpp>
#include <wrld/resources/Texture.hpp>

#include <glm/vec4.hpp>

#include <optional>

namespace wrld::rsc {
    enum PolygonMode {
        WrldPolyFill = 1 << 0,
        WrldPolyLine = 1 << 1,
        WrldPolyPoint = 1 << 2,
    };

    constexpr PolygonMode operator|(const PolygonMode a, const PolygonMode b) {
        return static_cast<PolygonMode>(static_cast<int>(a) | static_cast<int>(b));
    }

    class Material final : public Resource {
    public:
        Material(std::string name, World &world /*, Rc<Resource> *rc*/);

        Material(Material &other) = delete;

        Material(Material &&other) = delete;

        Material &operator=(Material &other) = delete;

        Material &operator=(Material &&other) = delete;

        Material &set_diffuse_map(const Rc<Texture> &diffuse_map);

        Material &set_specular_map(const Rc<Texture> &specular_map);

        void remove_diffuse_map();

        void remove_specular_map();

        [[nodiscard]] std::optional<Rc<Texture>> get_diffuse_map() const;

        [[nodiscard]] std::optional<Rc<Texture>> get_specular_map() const;

        Material &set_diffuse_color(const glm::vec4 &color);

        Material &set_specular_intensity(float intensity);

        [[nodiscard]] glm::vec4 get_diffuse_color() const;

        [[nodiscard]] float get_specular_intensity() const;

        Material &set_shininess(float shininess);

        [[nodiscard]] float get_shininess() const;

        /// If no diffuse map is specified, specify
        /// if we use Material::diffuse_color or the
        /// embedded colors in the mesh.
        void use_mesh_color(bool use);

        /// If set to False, the object won't be lit.
        /// (equivalent to a full white ambiant light).
        void do_lighting(bool do_lighting);

        /// If set to False, the object won't write to the
        /// depth mask.
        void do_depth_mask(bool do_depth_mask);

        [[nodiscard]] bool is_doing_depth_mask() const;

        [[nodiscard]] bool is_using_mesh_color() const;

        [[nodiscard]] bool is_doing_lighting() const;

        [[nodiscard]] PolygonMode get_polygon_mode() const;

        void set_polygon_mode(PolygonMode polygon_mode);

        [[nodiscard]] float get_line_width() const;

        void set_line_width(float line_width);

        [[nodiscard]] float get_point_size() const;

        void set_point_size(float point_size);

        std::string get_type() const override { return "Material"; }

    private:
        // Constant colors if maps are not specified
        glm::vec4 diffuse_color = {1.0, 1.0, 1.0, 1.0};
        float specular_intensity = 0.5;

        // Maps
        std::optional<Rc<Texture>> diffuse_map; // Diffuse color
        std::optional<Rc<Texture>> specular_map; // Specular intensity

        // todo: parameters that should be in the material
        // - depth mask (enable/disable)
        // - depth func
        // - enable/disable each type of light (ambiant, diffuse, specular)
        // - keep do_lighting. If the three are disabled, the object would be black
        // otherwise
        // - cull mode
        // - do depth test. If disable the object will be drawn on top of every others
        // - do projected shadows
        // - texture filtering mode (LINEAR, NEAREST, etc..)
        // others...

        // Physical values
        float shininess = 32;

        // Config
        bool _use_mesh_color = true;
        bool _do_lighting = true;
        bool _do_depth_mask = true;
        PolygonMode polygon_mode = WrldPolyFill;

        float line_width = 1.0;
        float point_size = 1.0;
    };
} // namespace wrld::rsc
