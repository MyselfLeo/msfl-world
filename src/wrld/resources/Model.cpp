//
// Created by leo on 8/15/25.
//

#include <wrld/resources/Model.hpp>
#include <wrld/World.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <assimp/postprocess.h>
#include <wrld/logs.hpp>
#include <wrld/objects/geometry/Triangle.hpp>

namespace wrld::rsc {
    Model::Model(std::string name, World &world) : Resource(std::move(name), world), vao(0), vbo(0), ebo(0) {
    }

    GeometryUsage Model::get_geometry_usage() const { return usage; }

    void Model::set_geometry_usage(const GeometryUsage usage) { this->usage = usage; }

    unsigned Model::add_material(const Rc<Material> &material) {
        const unsigned res = materials.size();
        materials.push_back(material);
        return res;
    }

    const std::vector<Rc<Material> > &Model::get_materials() const { return materials; }

    const std::vector<obj::MeshGroup> &Model::get_mesh_groups() const {
        return groups;
    }

    std::vector<std::pair<obj::MeshGroup, int> > Model::get_mesh_groups_and_materials() const {
        std::vector<std::pair<obj::MeshGroup, int> > res;
        res.reserve(groups.size());

        for (int i = 0; i < groups.size(); i++) {
            res.emplace_back(groups[i], material_of_group[i]);
        }

        return res;
    }

    Model &Model::from_mesh(const obj::Mesh &mesh, const Rc<Material> &material) {
        // Group with 1 mesh
        obj::MeshGroup group;
        group.add_mesh(mesh);

        return from_mesh_group(group, material);
    }

    Model &Model::from_mesh_group(const obj::MeshGroup &meshgroup,
                                  const Rc<Material> &material) {
        clear();

        int material_index;
        if (const int existing = get_material_index(material); existing != -1) {
            material_index = existing;
        } else {
            material_index = materials.size();
            materials.push_back(material);
        }

        groups.push_back(meshgroup);
        material_of_group.push_back(material_index);

        update();
        return *this;
    }

    Model &Model::from_mesh_groups(const std::vector<obj::MeshGroup> &meshgroups,
                                   const std::vector<Rc<Material> > &groups_materials) {
        if (meshgroups.size() != groups_materials.size()) {
            throw std::runtime_error("meshgroups.size() != groups_materials.size()");
        }

        clear();

        for (int i = 0; i < meshgroups.size(); i++) {
            int material_index;
            if (const int existing = get_material_index(groups_materials[i]);
                existing != -1) {
                material_index = existing;
            } else {
                material_index = materials.size();
                materials.push_back(groups_materials[i]);
            }

            groups.push_back(meshgroups[i]);
            material_of_group.push_back(material_index);
        }

        update();
        return *this;
    }

    Model &Model::from_file(const std::string &model_path, unsigned ai_flags,
                            bool flip_textures,
                            const std::optional<Rc<Material> > &custom_material) {
        wrldInfo(std::format("Loading model from {}", model_path).c_str());

        clear();

        const std::string model_directory =
                model_path.substr(0, model_path.find_last_of('/'));

        // Load Assimp scene from file
        Assimp::Importer import;
        const aiScene *scene =
                import.ReadFile(model_path, ai_flags | aiProcess_SortByPType);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            throw std::runtime_error(std::format("Unable to load model `{}`: {}",
                                                 model_path, import.GetErrorString()));
        }
        if (scene->mNumMeshes == 0) {
            return *this;
        }

        if (custom_material.has_value()) {
            materials.push_back(custom_material.value());
        } else {
            load_materials(scene, model_directory, flip_textures);
        }

        // Create 1 meshgroup for each mesh
        // todo: We never use more than 1 mesh per meshgroup so maybe
        // just forget about meshgroup ??
        for (int m = 0; m < scene->mNumMeshes; m++) {
            const aiMesh *mesh = scene->mMeshes[m];

            obj::PrimitiveType primitive_type;
            if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT)
                primitive_type = obj::PrimitiveType::Points;
            else if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE)
                primitive_type = obj::PrimitiveType::Lines;
            else if (mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
                primitive_type = obj::PrimitiveType::Triangles;
            else {
                wrldError(
                    std::format("Primitive type unspported in mesh {}", model_path));
                continue;
            }


            // Create the new mesh
            obj::Mesh new_mesh{primitive_type};

            // Process vertices
            for (unsigned i = 0; i < mesh->mNumVertices; i++) {
                obj::Vertex vertex;

                const aiVector3D &vertex_pos = mesh->mVertices[i];
                const aiVector3D &vertex_normal = mesh->mNormals[i];
                const aiVector3D &vertex_texcoords = mesh->mTextureCoords[0]
                                                         ? mesh->mTextureCoords[0][i]
                                                         : aiVector3D{0, 0, 0};
                const aiColor4D &vertex_color = mesh->mColors[0]
                                                    ? mesh->mColors[0][i]
                                                    : aiColor4D{1.0, 1.0, 1.0, 1.0};

                vertex.position = {vertex_pos.x, vertex_pos.y, vertex_pos.z};
                vertex.normal = {vertex_normal.x, vertex_normal.y, vertex_normal.z};
                vertex.color = {vertex_color.r, vertex_color.g, vertex_color.b};
                vertex.texcoords = {vertex_texcoords.x, vertex_texcoords.y};

                new_mesh.add_vertex(vertex);
            }
            // Indices
            for (unsigned i = 0; i < mesh->mNumFaces; i++) {
                const aiFace &face = mesh->mFaces[i];
                for (unsigned j = 0; j < face.mNumIndices; j++) {
                    new_mesh.add_element(face.mIndices[j]);
                }
            }

            // Create a meshgroup
            // See previous todo
            obj::MeshGroup new_meshgroup;
            new_meshgroup.add_mesh(new_mesh);

            // Add the meshgroup along with the material indice
            groups.push_back(new_meshgroup);
            material_of_group.push_back(
                custom_material.has_value() ? 0 : mesh->mMaterialIndex);
        }
        update();
        return *this;
    }

    size_t Model::get_mesh_count() const {
        size_t count = 0;
        for (const auto &g: groups) {
            count += g.get_mesh_count();
        }
        return count;
    }

    void Model::update() {
        update_bounding_box();

        // Collect vertices & elements to send to the GPU
        std::vector<obj::Vertex> vertices;
        std::vector<obj::VertexID> elements;
        vertices.reserve(get_vertex_count());
        elements.reserve(get_element_count());

        mesh_ebo_data.clear();

        // We just concatenate each MeshGroup data into one large
        for (const auto &[igroup, mgroup]: groups | std::views::enumerate) {
            const auto &agg = mgroup.get_aggregate();
            int vertices_offset = vertices.size();
            int elements_offset = elements.size();

            for (const auto &v: agg.vertices) {
                vertices.push_back(v);
            }
            for (const auto &e: agg.elements) {
                elements.push_back(e + vertices_offset);
            }

            // Build mesh_ebo_data
            const auto type = mgroup.get_primitive_type();
            const auto mat = material_of_group[igroup];

            if (!mesh_ebo_data.contains(type)) {
                mesh_ebo_data.insert_or_assign(
                    type, std::unordered_map<int, std::vector<MeshEBOData> >{});
            }
            if (!mesh_ebo_data.at(type).contains(mat)) {
                mesh_ebo_data.at(type).insert_or_assign(mat, std::vector<MeshEBOData>{});
            }

            mesh_ebo_data.at(type).at(mat).emplace_back(mgroup.get_element_count(),
                                                        elements_offset);
        }

        // Send data to the GPU
        if (vao == 0)
            glGenVertexArrays(1, &vao);
        if (vbo == 0)
            glGenBuffers(1, &vbo);
        if (ebo == 0)
            glGenBuffers(1, &ebo);

        GLenum gl_usage;
        switch (usage) {
            case GeometryUsage::Static: {
                gl_usage = GL_STATIC_DRAW;
            }
            break;
            case GeometryUsage::Dynamic: {
                gl_usage = GL_DYNAMIC_DRAW;
            }
            break;
            default: {
                std::unreachable();
            }
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(obj::Vertex),
                     vertices.data(), gl_usage);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned),
                     elements.data(), gl_usage);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              static_cast<void *>(nullptr));
        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, normal)));
        // Vertex colors
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, color)));
        // Vertex texture coordinates
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, texcoords)));
        glBindVertexArray(0);
    }

    const obj::AABoundingBox &Model::get_bounding_box() const { return bounding_box; }

    obj::BVHierarchy<obj::Triangle> Model::compute_bvh(const glm::mat4x4 &transform,
                                                       unsigned group_size) const {
        // We'll have to collect the triangles so we get the count first
        unsigned triangle_count = 0;
        for (const auto &g: groups) {
            for (const auto &m: g.get_meshes()) {
                if (m.get_primitive_type() != obj::PrimitiveType::Triangles)
                    continue;

                triangle_count += m.get_element_count() /
                        obj::get_primitive_size(obj::PrimitiveType::Triangles);
            }
        }

        std::vector<obj::Triangle> triangles;
        triangles.reserve(triangle_count);

        // Query the triangles
        for (const auto &g: groups) {
            for (const auto &m: g.get_meshes()) {
                if (m.get_primitive_type() != obj::PrimitiveType::Triangles)
                    continue;

                const auto &el = m.get_elements();
                const auto &vs = m.get_vertices();
                for (int i = 0; i < m.get_element_count(); i += 3) {
                    const auto v1 = transform * glm::vec4{vs[el[i + 0]].position, 1.0};
                    const auto v2 = transform * glm::vec4{vs[el[i + 1]].position, 1.0};
                    const auto v3 = transform * glm::vec4{vs[el[i + 2]].position, 1.0};


                    triangles.emplace_back(v1, v2, v3);
                }
            }
        }

        // Construct the BVH over the triangles
        return obj::BVHierarchy{triangles, group_size};
    }

    int Model::get_material_index(const Rc<Material> &material) const {
        // todo: currently it's O(n)
        for (const auto &[i, m]: materials | std::views::enumerate) {
            if (m == material) {
                return i;
            }
        }
        return -1;
    }

    void Model::update_bounding_box() {
        this->bounding_box = obj::AABoundingBox{};

        for (const auto &mg: groups) {
            for (const auto &m: mg.get_meshes()) {
                this->bounding_box = this->bounding_box + m.get_bounding_box();
            }
        }
    }

    unsigned Model::get_vertex_count() const {
        unsigned s = 0;
        for (const auto &m: groups) {
            s += m.get_vertex_count();
        }
        return s;
    }

    unsigned Model::get_element_count() const {
        unsigned s = 0;
        for (const auto &m: groups) {
            s += m.get_element_count();
        }
        return s;
    }

    void Model::load_materials(const aiScene *scene, const std::string &directory,
                               const bool flip_textures) {
        materials.clear();
        materials.reserve(scene->mNumMaterials);

        for (int i = 0; i < scene->mNumMaterials; i++) {
            // todo: load more data from the material, including :
            // - all the textures (not limited to 1 texture per type)
            // - PBR data

            // Create the material
            const aiMaterial *ai_material = scene->mMaterials[i];
            const auto material =
                    world.create_resource<Material>(ai_material->GetName().C_Str());

            // Load the textures
            if (ai_material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString str;
                ai_material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                const auto texture =
                        load_texture(directory, std::string(str.C_Str()), scene,
                                     aiTextureType_DIFFUSE, flip_textures);

                material->set_diffuse_map(texture);
            }
            if (ai_material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
                aiString str;
                ai_material->GetTexture(aiTextureType_SPECULAR, 0, &str);

                const auto texture =
                        load_texture(directory, std::string(str.C_Str()), scene,
                                     aiTextureType_SPECULAR, flip_textures);

                material->set_specular_map(texture);
            }

            materials.push_back(material);
        }
    }

    Rc<Texture> Model::load_texture(const std::string &directory, const std::string &str,
                                    const aiScene *scene, const aiTextureType &type,
                                    const bool flip_texture) {
        // Texture already loaded
        if (textures.contains(str))
            return textures.at(str);

        // Case of an embedded file
        if (scene->GetEmbeddedTexture(str.c_str())) {
            throw std::runtime_error("Embedded textures are not supported yet");
        }

        // Case of an external file
        const std::string texture_path = std::format("{}/{}", directory, str);

        // Load the texture
        auto texture = world.create_resource<Texture>(str);
        texture.get_mut()->from_file(texture_path, type, flip_texture);
        textures.insert_or_assign(str, texture);

        return texture;
    }

    void Model::clear() {
        textures.clear();
        materials.clear();
        groups.clear();
        material_of_group.clear();
    }

    const std::unordered_map<obj::PrimitiveType,
        std::unordered_map<int, std::vector<MeshEBOData> > > &
    Model::get_mesh_ebo_data() const {
        return mesh_ebo_data;
    }

    GLuint Model::get_vao() const { return vao; }

    // std::vector<Rc<Texture>> Model::load_textures(const aiMaterial *material,
    //                                               const aiTextureType type,
    //                                               const aiScene *scene,
    //                                               const bool flip_textures,
    //                                               const unsigned max) {
    //     const unsigned count = std::min(material->GetTextureCount(type), max);
    //
    //     std::vector<Rc<Texture>> res;
    //     res.reserve(count);
    //
    //     for (unsigned i = 0; i < count; i++) {
    //         // str can either be an embedded texture OR an external texture that will
    //         // be loaded from filesystem
    //         aiString str;
    //         material->GetTexture(type, i, &str);
    //
    //         // Case of an embedded file
    //         if (scene->GetEmbeddedTexture(str.C_Str())) {
    //             throw std::runtime_error("Embedded textures are not supported yet");
    //         }
    //
    //         // Case of an external file
    //         const std::string texture_path =
    //                 std::format("{}/{}", model_directory, str.C_Str());
    //
    //         // If it was already loaded, just return the cached structure
    //         if (loaded_textures.contains(texture_path)) {
    //             res.push_back(loaded_textures.at(texture_path));
    //             continue;
    //         }
    //
    //         // If not, load the texture to GPU, add it to cache and return
    //         auto texture = world.create_resource<Texture>(str.C_Str());
    //         texture.get_mut()->set_texture(texture_path, type, flip_textures);
    //         loaded_textures.insert_or_assign(texture_path, texture);
    //         res.push_back(texture);
    //     }
    //
    //     return res;
    // }
} // namespace wrld::rsc
