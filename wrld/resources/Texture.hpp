//
// Created by leo on 8/8/25.
//

#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include "assimp/material.h"


#include <string>

#include <glad/glad.h>

namespace wrld::rsc {
    class Texture {
    public:
        explicit Texture(const std::string &texture_path, aiTextureType type);

        Texture(Texture &&other) noexcept;
        Texture &operator=(Texture &&other) noexcept;

        void use(unsigned unit = 0) const;

        ~Texture();

    private:
        GLuint gl_texture;
        std::string path;

        // Using Assimp enum for now, it's good enough
        aiTextureType type;
    };
} // namespace wrld::rsc

#endif // TEXTURE_HPP
