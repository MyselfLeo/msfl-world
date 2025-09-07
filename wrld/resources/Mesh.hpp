//
// Created by leo on 9/6/25.
//

#ifndef MESH_HPP
#define MESH_HPP
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include "Material.hpp"
#include "Resource.hpp"

#include <memory>
#include <vector>

namespace wrld::rsc {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoords;
        glm::vec3 color;
    };

    typedef GLuint VertexID;
    typedef GLuint ElementID;

    class Mesh final : public Resource {
    public:
        explicit Mesh(ResourceID resource_id, World &world, const std::shared_ptr<Material> &default_material);

        Mesh(ResourceID resource_id, World &world, const std::shared_ptr<Material> &default_material,
             const std::vector<Vertex> &vertices, const std::vector<VertexID> &elements);

        Mesh(Mesh &other) = delete;
        Mesh(Mesh &&other) = delete;
        Mesh &operator=(Mesh &other) = delete;
        Mesh &operator=(Mesh &&other) = delete;

        ~Mesh() override;

        void set_material(const std::shared_ptr<Material> &material);
        void use_default_material();

        VertexID add_vertex(const Vertex &vertex);
        Vertex &get_vertex(VertexID vertex_id);

        ElementID add_element(VertexID vertex_id);
        VertexID &get_element(ElementID element_id);

        void set_gl_primitive_type(GLenum type);
        [[nodiscard]] GLenum get_gl_primitive_type() const;

        void set_gl_usage(GLenum usage);
        [[nodiscard]] GLenum get_gl_usage() const;

        [[nodiscard]] const std::shared_ptr<Material> &get_material() const;

        /// Sends/Updates mesh data on the GPU, setup vao/vbo/ebo is required
        void update();

        [[nodiscard]] GLuint get_vao() const;

        [[nodiscard]] unsigned get_element_count() const;

    private:
        bool buffers_created = false;

        std::vector<Vertex> vertices;
        std::vector<VertexID> indices;
        // std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

        std::shared_ptr<Material> default_material;
        std::shared_ptr<Material> current_material;

        GLenum gl_primitive_type = GL_TRIANGLES;
        GLenum gl_usage = GL_STATIC_DRAW;

        GLuint vao = 0, vbo = 0, ebo = 0;
    };

} // namespace wrld::rsc

#endif // MESH_HPP
