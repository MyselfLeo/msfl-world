//
// Created by leo on 8/15/25.
//

#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Texture.hpp"
#include "assimp/scene.h"

#include <memory>
#include <unordered_map>

namespace wrld {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoords;
    };

    class Mesh {
    public:
        Mesh() = default;
        Mesh(Mesh &&other) noexcept;
        Mesh &operator=(Mesh &&other) noexcept;
        ~Mesh();

    private:
        friend class Model;
        friend class RendererSystem;

        std::vector<Vertex> vertices;
        std::vector<unsigned> indices;
        std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

        GLuint vao = 0, vbo = 0, ebo = 0;

        /// Sends mesh data to the GPU, setup vao/vbo/ebo
        void init();
    };

    typedef size_t MeshID;

    class MeshGraphNode {
    public:
        MeshGraphNode() = default;
        MeshGraphNode(MeshGraphNode &&other) noexcept;
        MeshGraphNode &operator=(MeshGraphNode &&other) noexcept;

        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<MeshGraphNode>> children;
    };

    /// Stores multiple meshes in a tree representation
    class Model {
    public:
        explicit Model(const std::string &model_path);

        [[nodiscard]] size_t get_mesh_count() const;
        [[nodiscard]] const std::shared_ptr<MeshGraphNode> &get_root_mesh() const;

    private:
        friend class RendererSystem;

        std::shared_ptr<MeshGraphNode> root_mesh;
        size_t mesh_count;
        std::unordered_map<std::string, std::shared_ptr<Texture>> loaded_textures;

        // Save the directory where we loaded the model in order
        // to load relative textures
        std::string model_directory;

        std::shared_ptr<MeshGraphNode> process_node(const aiNode *node, const aiScene *scene);

        Mesh process_mesh(const aiMesh *mesh, const aiScene *scene);

        // Load textures of the given type from aiMaterial
        std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material, aiTextureType type,
                                                            const aiScene *scene);

        // todo: Maybe use 1 VAO/VBO/EBO for each model and not one for each mesh
        //       I'm not sure if it's worth it
        //       I won't over optimize for now
    };

} // namespace wrld

#endif // MODEL_HPP
