//
// Created by leo on 9/2/25.
//

#include <wrld/resources/Material.hpp>

namespace wrld::rsc {
    Material::Material(std::string name, World &world /*, Rc<Resource> *rc*/) :
        Resource(std::move(name), world /*, rc*/) {}

    Material &Material::set_diffuse_map(const Rc<Texture> &diffuse_map) {
        this->diffuse_map = diffuse_map;
        return *this;
    }

    Material &Material::set_specular_map(const Rc<Texture> &specular_map) {
        this->specular_map = specular_map;
        return *this;
    }

    void Material::remove_diffuse_map() { this->diffuse_map = std::nullopt; }

    void Material::remove_specular_map() { this->specular_map = std::nullopt; }

    std::optional<Rc<Texture>> Material::get_diffuse_map() const { return this->diffuse_map; }

    std::optional<Rc<Texture>> Material::get_specular_map() const { return this->specular_map; }

    Material &Material::set_diffuse_color(const glm::vec4 &color) {
        this->diffuse_color = color;
        return *this;
    }

    Material &Material::set_specular_intensity(const float intensity) {
        this->specular_intensity = intensity;
        return *this;
    }

    glm::vec4 Material::get_diffuse_color() const { return this->diffuse_color; }

    float Material::get_specular_intensity() const { return this->specular_intensity; }

    Material &Material::set_shininess(const float shininess) {
        this->shininess = shininess;
        return *this;
    }

    float Material::get_shininess() const { return this->shininess; }

    void Material::use_mesh_color(const bool use) { this->_use_mesh_color = use; }

    void Material::do_lighting(const bool do_lighting) { this->_do_lighting = do_lighting; }

    bool Material::is_using_mesh_color() const { return this->_use_mesh_color; }

    bool Material::is_doing_lighting() const { return this->_do_lighting; }
} // namespace wrld::rsc
