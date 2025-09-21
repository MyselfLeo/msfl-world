//
// Created by leo on 9/6/25.
//

#ifndef MESH_HPP
#define MESH_HPP
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <wrld/resources/Material.hpp>
#include <wrld/resources/Resource.hpp>

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

    class Mesh : public Resource {
    public:
        explicit Mesh(std::string name, World &world /*, Rc<Resource> *rc*/);

        Mesh(Mesh &other) = delete;
        Mesh(Mesh &&other) = delete;
        Mesh &operator=(Mesh &other) = delete;
        Mesh &operator=(Mesh &&other) = delete;

        ~Mesh() override;

        Mesh &set_material(const Rc<Material> &material);
        Mesh &set_vertices(const std::vector<Vertex> &vertices);
        Mesh &set_elements(const std::vector<VertexID> &elements);

        VertexID add_vertex(const Vertex &vertex);
        Vertex &get_vertex(VertexID vertex_id);

        ElementID add_element(VertexID vertex_id);
        VertexID &get_element(ElementID element_id);

        Mesh &set_gl_primitive_type(GLenum type);
        [[nodiscard]] GLenum get_gl_primitive_type() const;

        Mesh &set_gl_usage(GLenum usage);
        [[nodiscard]] GLenum get_gl_usage() const;

        [[nodiscard]] Rc<Material> get_material() const;

        /// Sends/Updates mesh data on the GPU, setup vao/vbo/ebo is required
        void update();

        [[nodiscard]] GLuint get_vao() const;

        [[nodiscard]] unsigned get_element_count() const;

        std::string get_type() const override { return "Mesh"; }

    private:
        bool buffers_created = false;

        std::vector<Vertex> vertices;
        std::vector<VertexID> indices;
        // std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

        // std::shared_ptr<const Material> current_material;

        GLenum gl_primitive_type = GL_TRIANGLES;
        GLenum gl_usage = GL_STATIC_DRAW;

        GLuint vao = 0, vbo = 0, ebo = 0;
    };

} // namespace wrld::rsc

#endif // MESH_HPP
