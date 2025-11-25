//
// Created by leo on 8/8/25.
//

#include <wrld/resources/Texture.hpp>
#include <wrld/logs.hpp>

#include <glad/glad.h>

#include <format>
#include <iostream>
#include <stb_image.hpp>
#include <stdexcept>

namespace wrld::rsc {
    Texture::Texture(const std::string &name, World &world) : Resource(name, world) {}

    Texture &Texture::from_file(const std::string &texture_path, const aiTextureType type,
                                const bool flip_textures) {
        stbi_set_flip_vertically_on_load(flip_textures);

        wrldInfo(std::format("Loading {} texture : {}", aiTextureTypeToString(type),
                             texture_path));

        // Load texture file
        int width, height, nb_channels;
        unsigned char *data =
                stbi_load(texture_path.c_str(), &width, &height, &nb_channels, 0);

        if (data == nullptr) {
            stbi_image_free(data);
            throw std::runtime_error(
                    std::format("Error while loading texture {}", texture_path));
        }

        update_texture(width, height, nb_channels, data);
        stbi_image_free(data);
        return *this;
    }

    Texture &Texture::from_default() {
        std::array<unsigned char, 17> DEFAULT_TEXTURE{
                "\377\000\357\377\000\000\000\377\000\000\000\377\377\000\357\377"};

        update_texture(2, 2, 4, DEFAULT_TEXTURE.data());
        return *this;
    }

    void Texture::use(const unsigned unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
    }

    Texture::~Texture() { glDeleteTextures(1, &gl_texture); }

    void Texture::update_texture(const int width, const int height, int nb_channels,
                                 unsigned char *data) {
        GLenum format;
        switch (nb_channels) {
            case 3: {
                format = GL_RGB;
            } break;
            case 4: {
                format = GL_RGBA;
            } break;
            default: {
                stbi_image_free(data);
                throw std::runtime_error(std::format(
                        "Only RGB and RGBA images are supported for now. Nbchannels: {}",
                        nb_channels));
            }
        }

        if (gl_texture == 0) {
            glGenTextures(1, &gl_texture);
        }

        glBindTexture(GL_TEXTURE_2D, gl_texture);

        // todo: move to material
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE,
                     data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
} // namespace wrld::rsc
