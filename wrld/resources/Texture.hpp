//
// Created by leo on 8/8/25.
//

#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include "Resource.hpp"
#include "assimp/material.h"


#include <string>

#include <glad/glad.h>

namespace wrld::rsc {
    class Texture final : public Resource {
    public:
        explicit Texture(const std::string &name, World &world, const std::string &texture_path, aiTextureType type,
                         bool flip_textures = false);

        Texture(Texture &other) = delete;
        Texture(Texture &&other) = delete;
        Texture &operator=(Texture &other) = delete;
        Texture &operator=(Texture &&other) = delete;

        void use(unsigned unit = 0) const;

        ~Texture() override;

    private:
        GLuint gl_texture;
        std::string path;
        bool flip_textures;

        // Using Assimp enum for now, it's good enough
        aiTextureType type;
    };
} // namespace wrld::rsc

#endif // TEXTURE_HPP
