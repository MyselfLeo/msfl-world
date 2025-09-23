//
// Created by leo on 8/15/25.
//

#include <wrld/resources/Model.hpp>
#include <wrld/World.hpp>
#include <wrld/logs.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace wrld::rsc {
    MeshGraphNode::MeshGraphNode(MeshGraphNode &&other) noexcept : meshes(std::move(other.meshes)) {
        children.reserve(other.children.size());
        for (auto &child: other.children) {
            children.push_back(std::move(child));
        }
        other.children.clear();
    }

    MeshGraphNode &MeshGraphNode::operator=(MeshGraphNode &&other) noexcept {
        if (this != &other) {
            meshes = std::move(other.meshes);
            children.clear();
            children.reserve(other.children.size());
            for (auto &child: other.children) {
                children.push_back(std::move(child));
            }
            other.children.clear();
        }
        return *this;
    }

    Model::Model(std::string name, World &world) :
        Resource(std::move(name), world), mesh_count(0), vao(0), vbo(0), ebo(0), ai_flags(0), flip_textures(false) {}

    Model &Model::from_file(const std::string &model_path, const unsigned ai_flags, const bool flip_textures,
                            const std::optional<Rc<Material>> &custom_material) {
        this->model_path = model_path;
        model_directory = model_path.substr(0, model_path.find_last_of('/'));
        this->ai_flags = ai_flags;
        this->flip_textures = flip_textures;
        this->custom_material = custom_material;

        reload_from_file();
        aggregate();
        return *this;
    }

    Model &Model::from_mesh(const Rc<Mesh> &mesh) {
        meshes.clear();
        root_mesh = std::make_shared<MeshGraphNode>();
        root_mesh->meshes.push_back(mesh);
        meshes.push_back(mesh);
        mesh_count = 1;

        loaded_materials.clear();
        loaded_materials.push_back(mesh.get()->get_material());

        aggregate();

        return *this;
    }

    void Model::reload_from_file() {
        wrldInfo(std::format("Loading model {}", model_path).c_str());

        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(model_path, ai_flags);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error(std::format("Unable to load model `{}`: {}", model_path, import.GetErrorString()));
        }
        if (scene->mNumMeshes == 0) {
            throw std::runtime_error(std::format("Model `{}` has no meshes", model_path));
        }

        loaded_materials.clear();
        if (!custom_material.has_value()) {
            loaded_materials = load_materials(scene);
        } else {
            loaded_materials.push_back(custom_material.value());
        }

        meshes.clear();
        root_mesh = process_node(scene->mRootNode, scene);
    }

    void Model::aggregate() {
        vertices.clear();
        elements.clear();
        meshes_start.clear();
        meshes_size.clear();
        // primitive_types.clear();
        material_meshes.clear();

        // Pre-allocate vectors
        meshes_start.reserve(meshes.size());
        meshes_size.reserve(meshes.size());
        // primitive_types.reserve(meshes.size());

        size_t total_vertex_size = 0;
        size_t total_element_size = 0;
        for (const auto &[i, m]: meshes | std::views::enumerate) {
            total_vertex_size += m.get_ref().vertices.size();
            total_element_size += m.get_ref().indices.size();

            const auto &mat = m.get_ref().get_material().get_ref();
            if (!material_meshes.contains(mat.get_name())) {
                material_meshes.insert_or_assign(mat.get_name(), std::vector<unsigned>());
            }

            material_meshes.at(mat.get_name()).push_back(i);
        }

        vertices.reserve(total_vertex_size);
        elements.reserve(total_element_size);

        // Aggregate
        for (const auto &m: meshes) {
            const auto &mesh = m.get_ref();

            meshes_start.push_back(elements.size());
            meshes_size.push_back(mesh.indices.size());
            // primitive_types.push_back(mesh.get_gl_primitive_type());

            for (const auto &e: mesh.indices) {
                elements.push_back(e + vertices.size());
            }

            vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
        }

        // Update VAO/VBO/EBO
        if (vao == 0)
            glGenVertexArrays(1, &vao);
        if (vbo == 0)
            glGenBuffers(1, &vbo);
        if (ebo == 0)
            glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned), elements.data(), GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void *>(nullptr));

        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, normal)));

        // Vertex colors
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, color)));


        // Vertex texture coordinates
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, texcoords)));

        glBindVertexArray(0);
    }

    std::vector<Rc<Material>> Model::load_materials(const aiScene *scene) {
        std::vector<Rc<Material>> res;
        res.reserve(scene->mNumMaterials);

        for (int i = 0; i < scene->mNumMaterials; i++) {
            // todo: load more data from the material

            // Create the material
            const aiMaterial *ai_material = scene->mMaterials[i];
            auto material = world.create_resource<Material>(ai_material->GetName().C_Str());

            // Load the textures
            const auto diffuse_textures = load_textures(ai_material, aiTextureType_DIFFUSE, scene, flip_textures, 1);
            const auto specular_textures = load_textures(ai_material, aiTextureType_SPECULAR, scene, flip_textures, 1);

            if (!diffuse_textures.empty())
                material.get_mut()->set_diffuse_map(diffuse_textures[0]);
            if (!specular_textures.empty())
                material.get_mut()->set_specular_map(specular_textures[0]);

            res.push_back(material);
        }

        return res;
    }

    size_t Model::get_mesh_count() const { return mesh_count; }

    const std::shared_ptr<MeshGraphNode> &Model::get_root_mesh() const { return root_mesh; }

    const std::vector<Rc<Material>> &Model::get_materials() const { return loaded_materials; }

    const std::vector<unsigned int> &Model::get_material_meshes(const std::string &mat_name) const {
        return material_meshes.at(mat_name);
    }

    const std::vector<size_t> &Model::get_meshes_start() const { return meshes_start; }

    const std::vector<size_t> &Model::get_meshes_size() const { return meshes_size; }

    const std::vector<Vertex> &Model::get_vertices() const { return vertices; }

    const std::vector<VertexID> &Model::get_elements() const { return elements; }

    // const std::vector<GLenum> &Model::get_primitive_types() const { return primitive_types; }

    GLuint Model::get_vao() const { return vao; }

    const std::vector<Rc<Mesh>> &Model::get_meshes() const { return meshes; }

    std::shared_ptr<MeshGraphNode> Model::process_node(const aiNode *node, const aiScene *scene) {
        auto wrld_node = std::make_shared<MeshGraphNode>();

        // One node can contain multiple meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            const auto &new_mesh = process_mesh(mesh);
            meshes.push_back(new_mesh);
            wrld_node->meshes.push_back(new_mesh);
        }

        // One node can have multiple children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            wrld_node->children.push_back(process_node(node->mChildren[i], scene));
        }

        return wrld_node;
    }

    Rc<Mesh> Model::process_mesh(const aiMesh *mesh) {
        auto new_mesh = world.create_resource<Mesh>(mesh->mName.C_Str());
        if (custom_material.has_value()) {
            new_mesh.get_mut()->set_material(custom_material.value());
        } else {
            new_mesh.get_mut()->set_material(loaded_materials[mesh->mMaterialIndex]);
        }

        // Process vertices
        for (unsigned i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            const aiVector3D &vertex_pos = mesh->mVertices[i];
            const aiVector3D &vertex_normal = mesh->mNormals[i];
            const aiVector3D &vertex_texcoords =
                    mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D{0, 0, 0};
            const aiColor4D &vertex_color = mesh->mColors[0] ? mesh->mColors[0][i] : aiColor4D{1.0, 1.0, 1.0, 1.0};

            vertex.position = {vertex_pos.x, vertex_pos.y, vertex_pos.z};
            vertex.normal = {vertex_normal.x, vertex_normal.y, vertex_normal.z};
            vertex.color = {vertex_color.r, vertex_color.g, vertex_color.b};
            vertex.texcoords = {vertex_texcoords.x, vertex_texcoords.y};

            new_mesh.get_mut()->add_vertex(vertex);
        }

        // Indices
        for (unsigned i = 0; i < mesh->mNumFaces; i++) {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned j = 0; j < face.mNumIndices; j++) {
                new_mesh.get_mut()->add_element(face.mIndices[j]);
            }
        }

        mesh_count += 1;

        // new_mesh.get_mut()->update();
        return new_mesh;
    }

    std::vector<Rc<Texture>> Model::load_textures(const aiMaterial *material, const aiTextureType type,
                                                  const aiScene *scene, const bool flip_textures, const unsigned max) {
        const unsigned count = std::min(material->GetTextureCount(type), max);

        std::vector<Rc<Texture>> res;
        res.reserve(count);

        for (unsigned i = 0; i < count; i++) {
            // str can either be an embedded texture OR an external texture that will be loaded from filesystem
            aiString str;
            material->GetTexture(type, i, &str);

            // Case of an embedded file
            if (scene->GetEmbeddedTexture(str.C_Str())) {
                throw std::runtime_error("Embedded textures are not supported yet");
            }

            // Case of an external file
            const std::string texture_path = std::format("{}/{}", model_directory, str.C_Str());

            // If it was already loaded, just return the cached structure
            if (loaded_textures.contains(texture_path)) {
                res.push_back(loaded_textures.at(texture_path));
                continue;
            }

            // If not, load the texture to GPU, add it to cache and return
            auto texture = world.create_resource<Texture>(str.C_Str());
            texture.get_mut()->set_texture(texture_path, type, flip_textures);
            loaded_textures.insert_or_assign(texture_path, texture);
            res.push_back(texture);
        }

        return res;
    }


} // namespace wrld::rsc
