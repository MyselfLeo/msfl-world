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

// todo: it may be easier to have subclasses "FileModel" (loaded from 3D file)
// and "MeshModel" (loaded from a mesh in memory)

namespace wrld::rsc {

    class MeshGraphNode {
    public:
        MeshGraphNode() = default;
        MeshGraphNode(MeshGraphNode &&other) noexcept;
        MeshGraphNode &operator=(MeshGraphNode &&other) noexcept;

        std::vector<std::shared_ptr<const Mesh>> meshes;
        std::vector<std::shared_ptr<MeshGraphNode>> children;
    };

    /// Stores multiple meshes in a tree representation
    class Model final : public Resource {
    public:
        explicit Model(std::string name, World &world);

        /// Loads model from file
        Model &from_file(const std::string &model_path, unsigned ai_flags = 0, bool flip_textures = false,
                         const std::optional<std::shared_ptr<const Material>> &custom_material = std::nullopt);

        /// Creates a Model with a single mesh
        Model &from_mesh(const std::shared_ptr<const Mesh> &mesh);

        [[nodiscard]] size_t get_mesh_count() const;
        [[nodiscard]] const std::shared_ptr<MeshGraphNode> &get_root_mesh() const;

        std::string get_type() override { return "Model"; }

    private:
        friend class RendererSystem;

        std::shared_ptr<MeshGraphNode> root_mesh;
        size_t mesh_count;

        ////// BELOW : Data & functions when model is loaded from file

        // Cache loaded textures
        std::unordered_map<std::string, std::shared_ptr<const Texture>> loaded_textures;
        // Loaded materials
        std::vector<std::shared_ptr<const Material>> loaded_materials;

        // Save the directory where we loaded the model in order
        // to load relative textures
        std::string model_directory;
        std::string model_path;
        unsigned ai_flags;
        bool flip_textures;
        std::optional<std::shared_ptr<const Material>> custom_material;

        void reload_from_file();

        std::vector<std::shared_ptr<const Material>> load_materials(const aiScene *scene);

        std::shared_ptr<MeshGraphNode> process_node(const aiNode *node, const aiScene *scene);

        std::shared_ptr<const Mesh> process_mesh(const aiMesh *mesh);

        /// Load textures of the given type from aiMaterial.
        /// Will only return a maximum of max textures.
        std::vector<std::shared_ptr<const Texture>> load_textures(const aiMaterial *material, aiTextureType type,
                                                                  const aiScene *scene, bool flip_textures,
                                                                  unsigned max = 1);

        // Load the first texture of the given type from aiMaterial
        // std::shared_ptr<Texture> load_texture(const aiMaterial *material, aiTextureType type, const aiScene *scene);


        // todo: Maybe use 1 VAO/VBO/EBO for each model and not one for each mesh
        //       I'm not sure if it's worth it
        //       I won't over optimize for now
    };

} // namespace wrld::rsc

#endif // MODEL_HPP
