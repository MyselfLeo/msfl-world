//
// Created by leo on 9/2/25.
//

#include "Material.hpp"

namespace wrld::rsc {
    Material::Material(std::string name, World &world) : Resource(std::move(name), world) {}

    void Material::set_diffuse_map(const std::shared_ptr<Texture> &diffuse_map) { this->diffuse_map = diffuse_map; }

    void Material::set_specular_map(const std::shared_ptr<Texture> &specular_map) { this->specular_map = specular_map; }

    void Material::remove_diffuse_map() { this->diffuse_map = std::nullopt; }

    void Material::remove_specular_map() { this->specular_map = std::nullopt; }

    std::optional<std::shared_ptr<Texture>> Material::get_diffuse_map() const { return this->diffuse_map; }

    std::optional<std::shared_ptr<Texture>> Material::get_specular_map() const { return this->specular_map; }

    void Material::set_diffuse_color(const glm::vec4 &color) { this->diffuse_color = color; }

    void Material::set_specular_intensity(const float intensity) { this->specular_intensity = intensity; }

    glm::vec4 Material::get_diffuse_color() const { return this->diffuse_color; }

    float Material::get_specular_intensity() const { return this->specular_intensity; }

    void Material::set_shininess(const float shininess) { this->shininess = shininess; }

    float Material::get_shininess() const { return this->shininess; }

    void Material::use_mesh_color(const bool use) { this->_use_mesh_color = use; }

    void Material::do_lighting(const bool do_lighting) { this->_do_lighting = do_lighting; }

    bool Material::is_using_mesh_color() const { return this->_use_mesh_color; }

    bool Material::is_doing_lighting() const { return this->_do_lighting; }
} // namespace wrld::rsc
