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

        /// Loads the model with a single MeshGroup. You must give the material of this
        /// group.
        Model &from_mesh_group(const obj::MeshGroup &meshgroup,
                               const Rc<Material> &material);

        /// Loads the model with multiple MeshGroups and their associated materials.
        Model &from_mesh_groups(const std::vector<obj::MeshGroup> &meshgroups,
                                const std::vector<Rc<Material>> &groups_materials);

        /// Return the number of meshes in this model.
        [[nodiscard]] size_t get_mesh_count() const;

        /// Returns the geometry usage of this model. See Model::set_geometry_usage.
        [[nodiscard]] GeometryUsage get_geometry_usage() const;

        /// Defines the usage of this model. May allow for optimize memory location.
        /// Depends on the GPU and the driver.
        void set_geometry_usage(GeometryUsage usage);

        /// Add a material to this model. Returns its index in the model.
        unsigned add_material(const Rc<Material> &material);

        /// Return all the materials used in this model.
        const std::vector<Rc<Material>> &get_materials() const;

        /// Return the mesh groups of this Model alongside the index of their material.
        std::vector<std::pair<obj::MeshGroup, int>> get_mesh_groups() const;

        /// Update the model's data :
        /// - Mesh data is collected & sent to the GPU
        /// - the bounding box is updated
        /// This function should be called everytime a mesh is updated
        /// or added/removed.
        void update();

        /// Return the bounding box of this model in local space.
        const obj::Box &get_bounding_box() const;

        std::string get_type() const override { return "Model"; }

    protected:
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

        /// Load materials from the aiScene.
        void load_materials(const aiScene *scene, const std::string &directory,
                            bool flip_textures);

        // /// Load textures of the given type from aiMaterial.
        // /// Will only load a maximum of max textures.
        // void load_textures(const std::string &model_directory, const aiMaterial
        // *material,
        //                    aiTextureType type, const aiScene *scene, bool
        //                    flip_textures, unsigned max = 1);

        /// Load a specific texture based on its string.
        Rc<Texture> load_texture(const std::string &directory, const std::string &str,
                                 const aiScene *scene, const aiTextureType &type,
                                 bool flip_texture);

        /// Clear internal data of the Model.
        void clear();

        /// See wrld::obj::GeometryUsage.
        /// Might or might not be used by the driver.
        GeometryUsage usage = GeometryUsage::Static;

        std::unordered_map<std::string, Rc<Texture>> textures;
        std::vector<Rc<Material>> materials;
        std::vector<obj::MeshGroup> groups;

        /// Tells for each MeshGroup which material to use.
        std::vector<int> material_of_group;

        /// For each (material, primitive type) couple, lists
        /// memory data of each meshgroup using it.
        std::unordered_map<obj::PrimitiveType,
                           std::unordered_map<int, std::vector<MeshEBOData>>>
                mesh_ebo_data;

    public:
        [[nodiscard]] const std::unordered_map<
                obj::PrimitiveType, std::unordered_map<int, std::vector<MeshEBOData>>> &
        get_mesh_ebo_data() const;
        [[nodiscard]] GLuint get_vao() const;

    protected:
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
