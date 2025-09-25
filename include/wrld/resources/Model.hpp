//
// Created by leo on 8/15/25.
//

#pragma once

#include "assimp/scene.h"

#include <wrld/resources/Mesh.hpp>
#include <wrld/resources/Texture.hpp>
#include <wrld/tools/ModelTool.hpp>

#include <memory>
#include <unordered_map>
#include <vector>

// todo: it may be easier to have subclasses "FileModel" (loaded from 3D file)
// and "MeshModel" (loaded from a mesh in memory)

namespace wrld::rsc {
    /// Axis-aligned box (in a particular space).
    struct BoundingBox {
        glm::vec3 lower;
        glm::vec3 upper;
    };

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
        explicit Model(std::string name, World &world);

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

        const std::vector<unsigned> &get_material_meshes(const std::string &mat_name) const;

        const std::vector<size_t> &get_meshes_start() const;

        const std::vector<size_t> &get_meshes_size() const;

        const std::vector<Vertex> &get_vertices() const;

        const std::vector<VertexID> &get_elements() const;

        /// Query all meshes, build VAOs and bounding boxes
        void aggregate();

        /// Return the bounding box of this model in local space.
        const BoundingBox &get_local_bb() const;

        GLuint get_vao() const;

    private:
        friend class RendererSystem;
        friend class ModelTool;

        // todo: we store mesh references twice (in MeshGraphNode and in meshes)
        std::shared_ptr<MeshGraphNode> root_mesh;
        std::vector<Rc<Mesh>> meshes;
        size_t mesh_count;

        // todo: aggregate mesh data in one
        // question: are Meshes as resource even pertinent?
        GLuint vao, vbo, ebo;
        std::vector<Vertex> vertices;
        std::vector<VertexID> elements;
        std::vector<size_t> meshes_start; // Start position of the meshes in the EBO
        std::vector<size_t> meshes_size; // in vertices

        // For each material, list the ids of the meshes using it.
        std::unordered_map<std::string, std::vector<unsigned>> material_meshes;

        // Bounding box of the model in local space. Updated by Model::aggregate
        BoundingBox local_bb;

        // std::vector<Rc<Material>> meshes_materials; // Material of each mesh

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

        GLenum gl_primitive_type = GL_TRIANGLES;
        GLenum gl_usage = GL_STATIC_DRAW;

        void reload_from_file();

        /// Compute local bounding box of this mesh.
        BoundingBox compute_local_bb() const;

        std::vector<Rc<Material>> load_materials(const aiScene *scene);

        std::shared_ptr<MeshGraphNode> process_node(const aiNode *node, const aiScene *scene);

        Rc<Mesh> process_mesh(const aiMesh *mesh);

        /// Load textures of the given type from aiMaterial.
        /// Will only return a maximum of max textures.
        std::vector<Rc<Texture>> load_textures(const aiMaterial *material, aiTextureType type, const aiScene *scene,
                                               bool flip_textures, unsigned max = 1);
    };
} // namespace wrld::rsc
