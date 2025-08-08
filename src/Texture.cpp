//
// Created by leo on 8/8/25.
//

#include "Texture.hpp"

#include <format>
#include <stb_image.hpp>
#include <stdexcept>


namespace wrld {
    Texture::Texture(const std::string &texture_path) {
        // Load texture file
        int width, heigth, nb_channels;
        unsigned char *data = stbi_load(texture_path.c_str(), &width, &heigth, &nb_channels, 0);

        if (data == nullptr) {
            throw std::runtime_error(std::format("Error while loading texture {}", texture_path));
        }

        if (nb_channels != 3) {
            throw std::runtime_error("Only RGB images are supported for now");
        }

        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, heigth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }

    void Texture::use() const {
        glBindTexture(GL_TEXTURE_2D, gl_texture);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &gl_texture);
    }
} // wrld
