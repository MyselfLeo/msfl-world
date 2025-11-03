//
// Created by leo on 8/15/25.
//

#pragma once

#include "assimp/scene.h"

#include <memory>
#include <vector>

#include <wrld/objects/geometry/Box.hpp>
#include <wrld/objects/geometry/MeshGroup.hpp>
#include <wrld/objects/geometry/Mesh.hpp>
#include <wrld/resources/Texture.hpp>
#include <wrld/resources/Material.hpp>

namespace wrld::rsc {
    /// Defines the update frequency rate of this model's geometry.
    /// Static : Geometry is rarely updated.
    /// Dynamic : Geometry is frequently updated.
    enum class GeometryUsage {
        Static,
        Dynamic,
    };

    /// Info on the memory layout of each MeshGroup.
    struct MeshEBOData {
        int count; // Number of elements
        int start; // First index in the EBO
    };

    // class MeshGraphNode {
    // public:
    //     MeshGraphNode() = default;
    //
    //     MeshGraphNode(MeshGraphNode &&other) noexcept;
    //
    //     MeshGraphNode &operator=(MeshGraphNode &&other) noexcept;
    //
    //     std::vector<Rc<Mesh>> meshes;
    //     std::vector<std::shared_ptr<MeshGraphNode>> children;
    // };

    /// Stores multiple meshes in a tree representation
    class Model final : public Resource {
    public:
        explicit Model(std::string name, World &world);

        /// Loads a model from a file (.obj...).
        /// You can set a custom material for the newly created model.
        /// If defined, it will replace all materials in the file with the one given.
        Model &
        from_file(const std::string &model_path, unsigned ai_flags = 0,
                  bool flip_textures = false,
                  const std::optional<Rc<Material>> &custom_material = std::nullopt);

        /// Loads the model with a single Mesh. You must give the material of this mesh.
        Model &from_mesh(const obj::Mesh &mesh, const Rc<Material> &material);

        /// Loads the model with a signel MeshGroup. You must give the material of this
        /// group.
        Model &from_mesh_group(const obj::MeshGroup &meshgroup,
                               const Rc<Material> &material);

        /// Return the number of meshes in this model.
        [[nodiscard]] size_t get_mesh_count() const;

        [[nodiscard]] GeometryUsage get_geometry_usage() const;

        void set_geometry_usage(GeometryUsage usage);

        // /// Return all the materials used in this model.
        // const std::vector<Rc<Material>> &get_materials() const;

        // /// Return all the meshes in this mesh. They are grouped into MeshGroups.
        // /// All meshes in a group share the same PrimitiveType and the same Material.
        // const std::vector<obj::MeshGroup> &get_meshes() const;

        // [[nodiscard]] const std::shared_ptr<MeshGraphNode> &get_root_mesh() const;

        // const std::vector<unsigned> &
        // get_material_meshes(const std::string &mat_name) const;

        // const std::vector<size_t> &get_meshes_start() const;
        //
        // const std::vector<size_t> &get_meshes_size() const;
        //
        // const std::vector<Vertex> &get_vertices() const;
        //
        // const std::vector<VertexID> &get_elements() const;

        /// Update the model's data :
        /// - Mesh data is collected & sent to the GPU
        /// - the bounding box is updated
        /// This function should be called everytime a mesh is updated
        /// or added/removed.
        void update();

        /// Return the bounding box of this model in local space.
        const obj::Box &get_bounding_box() const;

        std::string get_type() const override { return "Model"; }

    private:
        friend class RendererSystem;
        // friend class ModelTool;

        /// Rebuild the bounding box of the model by iterating
        /// over every vertices.
        void update_bounding_box(const std::vector<obj::Vertex> &vertices);

        /// Returns the index of the material if it's already
        /// in Model::materials. Returns -1 otherwise.
        int get_material_index(const Rc<Material> &material) const;

        /// Return the number of vertices.
        [[nodiscard]] unsigned get_vertex_count() const;

        /// Return the number of element indices.
        [[nodiscard]] unsigned get_element_count() const;

        /// See wrld::obj::GeometryUsage.
        /// Might or might not be used by the driver.
        GeometryUsage usage = GeometryUsage::Static;

        std::vector<Rc<Material>> materials;
        std::vector<obj::MeshGroup> groups;

        /// Tells for each MeshGroup which material to use.
        std::vector<int> material_of_group;

        /// For each (material, primitive type) couple, lists
        /// memory data of each meshgroup using it.
        std::unordered_map<obj::PrimitiveType,
                           std::unordered_map<int, std::vector<MeshEBOData>>>
                mesh_ebo_data;

        GLuint vao, vbo, ebo;

        /// Bounding box of the model in local-space. Updated by Model::update
        obj::Box bounding_box;

        // std::vector<Rc<Material>> meshes_materials; // Material of each mesh

        ////// BELOW : Data & functions when model is loaded from file

        // Cache loaded textures
        // std::unordered_map<std::string, Rc<Texture>> loaded_textures;
        // Loaded materials
        // std::vector<Rc<Material>> loaded_materials;

        // Save the directory where we loaded the model in order
        // to load relative textures
        // std::string model_directory;
        // std::string model_path;
        // unsigned ai_flags;
        // bool flip_textures;
        // std::optional<Rc<Material>> custom_material;

        // GLenum gl_primitive_type = GL_TRIANGLES;
        // GLenum gl_usage = GL_STATIC_DRAW;

        // void reload_from_file();

        // std::vector<Rc<Material>> load_materials(const aiScene *scene);
        //
        // std::shared_ptr<MeshGraphNode> process_node(const aiNode *node,
        //                                             const aiScene *scene);
        //
        // Rc<Mesh> process_mesh(const aiMesh *mesh);

        // /// Load textures of the given type from aiMaterial.
        // /// Will only return a maximum of max textures.
        // std::vector<Rc<Texture>> load_textures(const aiMaterial *material,
        //                                        aiTextureType type, const aiScene
        //                                        *scene, bool flip_textures, unsigned max
        //                                        = 1);
    };
} // namespace wrld::rsc
