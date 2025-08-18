//
// Created by leo on 8/15/25.
//

#include "Model.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <ranges>

namespace wrld {
    void Mesh::init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices), indices.data(), GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void *>(nullptr));

        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, normal)));

        // Vertex texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, texcoords)));

        glBindVertexArray(0);
    }

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

    Mesh::Mesh(Mesh &&other) noexcept :
        vertices(std::move(other.vertices)), indices(std::move(other.indices)), textures(std::move(other.textures)),
        vao(other.vao), vbo(other.vbo), ebo(other.ebo) {
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept {
        if (this != &other) {
            if (vao != 0)
                glDeleteVertexArrays(1, &vao);
            if (vbo != 0)
                glDeleteBuffers(1, &vbo);
            if (ebo != 0)
                glDeleteBuffers(1, &ebo);

            // Transfer resources
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            textures = std::move(other.textures);
            vao = other.vao;
            vbo = other.vbo;
            ebo = other.ebo;

            // Reset source object
            other.vao = 0;
            other.vbo = 0;
            other.ebo = 0;
        }
        return *this;
    }

    Mesh::~Mesh() {
        glBindVertexArray(0);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    Model::Model(const std::string &model_path) : mesh_count(0) {
        Assimp::Importer import;
        const aiScene *scene =
                import.ReadFile(model_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw std::runtime_error(std::format("Unable to load model `{}`: {}", model_path, import.GetErrorString()));
        }

        model_directory = model_path.substr(0, model_path.find_last_of('/'));

        root_mesh = process_node(scene->mRootNode, scene);
    }

    size_t Model::get_mesh_count() const { return mesh_count; }

    const std::shared_ptr<MeshGraphNode> &Model::get_root_mesh() const { return root_mesh; }

    std::shared_ptr<MeshGraphNode> Model::process_node(const aiNode *node, const aiScene *scene) {
        std::shared_ptr<MeshGraphNode> wrld_node = std::make_shared<MeshGraphNode>();

        // One node can contain multiple meshes
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            wrld_node->meshes.push_back(process_mesh(mesh, scene));
        }

        // One node can have multiple children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            wrld_node->children.push_back(process_node(node->mChildren[i], scene));
        }

        return wrld_node;
    }

    Mesh Model::process_mesh(const aiMesh *mesh, const aiScene *scene) {
        Mesh new_mesh;

        // Process vertices
        for (unsigned i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;

            const aiVector3D &vertex_pos = mesh->mVertices[i];
            const aiVector3D &vertex_normal = mesh->mNormals[i];
            const aiVector3D &vertex_texcoords =
                    mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D{0, 0, 0};

            vertex.position = {vertex_pos.x, vertex_pos.y, vertex_pos.z};
            vertex.normal = {vertex_normal.x, vertex_normal.y, vertex_normal.z};
            vertex.texcoords = {vertex_texcoords.x, vertex_texcoords.y};

            new_mesh.vertices.push_back(vertex);
        }

        // Indices
        for (unsigned i = 0; i < mesh->mNumFaces; i++) {
            const aiFace &face = mesh->mFaces[i];
            for (unsigned j = 0; j < face.mNumIndices; j++) {
                new_mesh.indices.push_back(face.mIndices[j]);
            }
        }

        // Materials
        // there is always at least one material thanks to AI_SCENE_FLAGS_INCOMPLETE
        const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // Load each texture type into the mesh
        for (auto const [i, t]: std::views::enumerate(load_textures(material, aiTextureType_DIFFUSE, scene))) {
            new_mesh.textures.insert_or_assign(std::format("material_texture_diffuse{}", i), t);
        }
        for (auto const [i, t]: std::views::enumerate(load_textures(material, aiTextureType_SPECULAR, scene))) {
            new_mesh.textures.insert_or_assign(std::format("material_texture_specular{}", i), t);
        }

        mesh_count += 1;

        new_mesh.init();
        return new_mesh;
    }

    std::vector<std::shared_ptr<Texture>> Model::load_textures(const aiMaterial *material, aiTextureType type,
                                                               const aiScene *scene) {
        std::vector<std::shared_ptr<Texture>> res;

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
            auto texture = std::make_shared<Texture>(texture_path, type);
            loaded_textures.insert_or_assign(texture_path, texture);
            res.push_back(texture);
        }

        return res;
    }


} // namespace wrld
