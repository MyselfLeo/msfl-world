//
// Created by leo on 8/8/25.
//

#include "Texture.hpp"

#include <format>
#include <iostream>
#include <stb_image.hpp>
#include <stdexcept>

namespace wrld {
    Texture::Texture(const std::string &texture_path, const aiTextureType type) :
        gl_texture(0), path(texture_path), type(type) {
        stbi_set_flip_vertically_on_load(true);

        // Load texture file
        int width, height, nb_channels;
        unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nb_channels, 0);

        if (data == nullptr) {
            stbi_image_free(data);
            throw std::runtime_error(std::format("Error while loading texture {}", texture_path));
        }

        if (nb_channels != 3) {
            stbi_image_free(data);
            throw std::runtime_error("Only RGB images are supported for now");
        }

        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    Texture::Texture(Texture &&other) noexcept :
        gl_texture(other.gl_texture), path(std::move(other.path)), type(other.type) {
        other.gl_texture = 0;
    }

    Texture &Texture::operator=(Texture &&other) noexcept {
        if (gl_texture != 0)
            glDeleteTextures(1, &gl_texture);

        gl_texture = other.gl_texture;
        path = std::move(other.path);
        type = other.type;

        other.gl_texture = 0;

        return *this;
    }

    void Texture::use(const unsigned unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
    }

    Texture::~Texture() { glDeleteTextures(1, &gl_texture); }
} // namespace wrld
