//
// Created by leo on 8/15/25.
//

#ifndef MODEL_HPP
#define MODEL_HPP

#include "Mesh.hpp"


#include <vector>

#include "Texture.hpp"
#include "assimp/scene.h"

#include <memory>
#include <unordered_map>

namespace wrld::rsc {

    class MeshGraphNode {
    public:
        MeshGraphNode() = default;
        MeshGraphNode(MeshGraphNode &&other) noexcept;
        MeshGraphNode &operator=(MeshGraphNode &&other) noexcept;

        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<MeshGraphNode>> children;
    };

    /// Stores multiple meshes in a tree representation
    class Model final : public Resource {
    public:
        /// Loads model from file
        explicit Model(std::string name, World &world, const std::string &model_path);

        /// Creates a Model with a single mesh
        explicit Model(std::string name, World &world, const std::shared_ptr<Mesh> &mesh);

        [[nodiscard]] size_t get_mesh_count() const;
        [[nodiscard]] const std::shared_ptr<MeshGraphNode> &get_root_mesh() const;

        std::string get_type() override { return "Model"; }

    private:
        friend class RendererSystem;

        std::shared_ptr<MeshGraphNode> root_mesh;
        size_t mesh_count;
        std::unordered_map<std::string, std::shared_ptr<Texture>> loaded_textures;

        ////// BELOW : Data & functions when model is loaded from file

        // Save the directory where we loaded the model in order
        // to load relative textures
        std::string model_directory;

        std::shared_ptr<MeshGraphNode> process_node(const aiNode *node, const aiScene *scene);

        std::shared_ptr<Mesh> process_mesh(const aiMesh *mesh, const aiScene *scene);

        // Load textures of the given type from aiMaterial
        std::vector<std::shared_ptr<Texture>> load_textures(const aiMaterial *material, aiTextureType type,
                                                            const aiScene *scene);

        // Load the first texture of the given type from aiMaterial
        // std::shared_ptr<Texture> load_texture(const aiMaterial *material, aiTextureType type, const aiScene *scene);


        // todo: Maybe use 1 VAO/VBO/EBO for each model and not one for each mesh
        //       I'm not sure if it's worth it
        //       I won't over optimize for now
    };

} // namespace wrld::rsc

#endif // MODEL_HPP
