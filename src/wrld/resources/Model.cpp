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

    Model::Model(std::string name, World &world /*, Rc<Resource> *rc*/) :
        Resource(std::move(name), world), mesh_count(0), ai_flags(0), flip_textures(false) {}

    Model &Model::from_file(const std::string &model_path, unsigned ai_flags, bool flip_textures,
                            const std::optional<Rc<Material>> &custom_material) {
        this->model_path = model_path;
        model_directory = model_path.substr(0, model_path.find_last_of('/'));
        this->ai_flags = ai_flags;
        this->flip_textures = flip_textures;
        this->custom_material = custom_material;

        reload_from_file();
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

        new_mesh.get_mut()->update();
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
