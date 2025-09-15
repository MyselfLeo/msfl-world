//
// Created by leo on 8/15/25.
//

#include "Model.hpp"

#include "World.hpp"
#include "logs.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <ranges>
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
        Resource(std::move(name), world), mesh_count(0), ai_flags(0), flip_textures(false) {}

    Model &Model::from_file(const std::string &model_path, const unsigned ai_flags, const bool flip_textures) {
        this->model_path = model_path;
        model_directory = model_path.substr(0, model_path.find_last_of('/'));
        this->ai_flags = ai_flags;
        this->flip_textures = flip_textures;

        reload_from_file();
        return *this;
    }

    Model &Model::from_mesh(const std::shared_ptr<const Mesh> &mesh) {
        root_mesh = std::make_shared<MeshGraphNode>();
        root_mesh->meshes.push_back(mesh);

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

        root_mesh = process_node(scene->mRootNode, scene, flip_textures);
    }

    size_t Model::get_mesh_count() const { return mesh_count; }

    const std::shared_ptr<MeshGraphNode> &Model::get_root_mesh() const { return root_mesh; }

    std::shared_ptr<MeshGraphNode> Model::process_node(const aiNode *node, const aiScene *scene,
                                                       const bool flip_textures) {
        auto wrld_node = std::make_shared<MeshGraphNode>();

        // One node can contain multiple meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            wrld_node->meshes.push_back(process_mesh(mesh, scene, flip_textures));
        }

        // One node can have multiple children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            wrld_node->children.push_back(process_node(node->mChildren[i], scene, flip_textures));
        }

        return wrld_node;
    }

    std::shared_ptr<const Mesh> Model::process_mesh(const aiMesh *mesh, const aiScene *scene,
                                                    const bool flip_textures) {
        // Get the mesh material first
        // there is always at least one material thanks to AI_SCENE_FLAGS_INCOMPLETE
        // todo: load materials first, assign them to meshes later
        // this will prevent resource duplication
        const aiMaterial *ai_material = scene->mMaterials[mesh->mMaterialIndex];
        const auto mesh_material = world.create_resource<Material>("material");

        // We take the first texture of each type
        const auto diffuse_textures = load_textures(ai_material, aiTextureType_DIFFUSE, scene, flip_textures);
        const auto specular_textures = load_textures(ai_material, aiTextureType_SPECULAR, scene, flip_textures);

        if (!diffuse_textures.empty())
            mesh_material->set_diffuse_map(diffuse_textures[0]);
        if (!specular_textures.empty())
            mesh_material->set_specular_map(specular_textures[0]);

        auto new_mesh = world.create_resource<Mesh>("mesh");
        new_mesh->set_material(mesh_material);

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

            new_mesh->add_vertex(vertex);
        }

        // Indices
        for (unsigned i = 0; i < mesh->mNumFaces; i++) {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned j = 0; j < face.mNumIndices; j++) {
                new_mesh->add_element(face.mIndices[j]);
            }
        }

        mesh_count += 1;

        new_mesh->update();
        return new_mesh;
    }

    std::vector<std::shared_ptr<const Texture>> Model::load_textures(const aiMaterial *material,
                                                                     const aiTextureType type, const aiScene *scene,
                                                                     const bool flip_textures) {
        std::vector<std::shared_ptr<const Texture>> res;

        for (unsigned i = 0; i < material->GetTextureCount(type); i++) {
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
            auto texture = world.create_resource<Texture>("texture");
            texture->set_texture(texture_path, type, flip_textures);
            loaded_textures.insert_or_assign(texture_path, texture);
            res.push_back(texture);
        }

        return res;
    }


} // namespace wrld::rsc
