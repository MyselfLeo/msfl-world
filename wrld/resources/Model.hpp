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

        std::vector<Rc<Mesh>> meshes;
        std::vector<std::shared_ptr<MeshGraphNode>> children;
    };

    /// Stores multiple meshes in a tree representation
    class Model final : public Resource {
    public:
        explicit Model(std::string name, World &world /*, Rc<Resource> *rc*/);

        /// Loads model from file
        Model &from_file(const std::string &model_path, unsigned ai_flags = 0, bool flip_textures = false,
                         const std::optional<Rc<Material>> &custom_material = std::nullopt);

        /// Creates a Model with a single mesh
        Model &from_mesh(const Rc<Mesh> &mesh);

        [[nodiscard]] size_t get_mesh_count() const;
        [[nodiscard]] const std::shared_ptr<MeshGraphNode> &get_root_mesh() const;

        std::string get_type() const override { return "Model"; }

        const std::vector<Rc<Material>> &get_materials() const;
        const std::vector<Rc<Mesh>> &get_meshes() const;


    private:
        friend class RendererSystem;

        // todo: we store mesh references twice (in MeshGraphNode and in meshes)
        std::shared_ptr<MeshGraphNode> root_mesh;
        std::vector<Rc<Mesh>> meshes;
        size_t mesh_count;

        ////// BELOW : Data & functions when model is loaded from file

        // Cache loaded textures
        std::unordered_map<std::string, Rc<Texture>> loaded_textures;
        // Loaded materials
        std::vector<Rc<Material>> loaded_materials;

        // Save the directory where we loaded the model in order
        // to load relative textures
        std::string model_directory;
        std::string model_path;
        unsigned ai_flags;
        bool flip_textures;
        std::optional<Rc<Material>> custom_material;

        void reload_from_file();

        std::vector<Rc<Material>> load_materials(const aiScene *scene);

        std::shared_ptr<MeshGraphNode> process_node(const aiNode *node, const aiScene *scene);

        Rc<Mesh> process_mesh(const aiMesh *mesh);

        /// Load textures of the given type from aiMaterial.
        /// Will only return a maximum of max textures.
        std::vector<Rc<Texture>> load_textures(const aiMaterial *material, aiTextureType type, const aiScene *scene,
                                               bool flip_textures, unsigned max = 1);
    };

} // namespace wrld::rsc

#endif // MODEL_HPP
