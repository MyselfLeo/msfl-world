//
// Created by leo on 8/8/25.
//

#include "Texture.hpp"

#include "logs.hpp"

#include <format>
#include <iostream>
#include <stb_image.hpp>
#include <stdexcept>

namespace wrld::rsc {
    Texture::Texture(const std::string &name, World &world/*, const std::string &texture_path, const aiTextureType type,
                     const bool flip_textures*/) :
        Resource(name, world) {
        reload();
    }

    Texture &Texture::set_texture(const std::string &texture_path, const aiTextureType type, const bool flip_textures) {
        this->path = texture_path;
        this->type = type;
        this->flip_textures = flip_textures;
        reload();
        return *this;
    }

    // Texture::Texture(Texture &&other) noexcept :
    //     gl_texture(other.gl_texture), path(std::move(other.path)), type(other.type),
    //     flip_textures(other.flip_textures) {
    //     other.gl_texture = 0;
    // }
    //
    // Texture &Texture::operator=(Texture &&other) noexcept {
    //     if (gl_texture != 0)
    //         glDeleteTextures(1, &gl_texture);
    //
    //     gl_texture = other.gl_texture;
    //     path = std::move(other.path);
    //     type = other.type;
    //     flip_textures = other.flip_textures;
    //
    //     other.gl_texture = 0;
    //
    //     return *this;
    // }

    void Texture::use(const unsigned unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, gl_texture);
    }

    Texture::~Texture() { glDeleteTextures(1, &gl_texture); }

    void Texture::reload() {
        stbi_set_flip_vertically_on_load(flip_textures);

        wrldInfo(std::format("Loading {} texture : {}", aiTextureTypeToString(type), path));

        // Filtering for regular textures
        // todo: move to material
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Load texture file
        int width, height, nb_channels;
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &nb_channels, 0);

        if (data == nullptr) {
            stbi_image_free(data);
            throw std::runtime_error(std::format("Error while loading texture {}", path));
        }

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
                throw std::runtime_error(
                        std::format("Only RGB and RGBA images are supported for now. Nbchannels: {}", nb_channels));
            }
        }

        if (gl_texture == 0) {
            glGenTextures(1, &gl_texture);
        }

        glBindTexture(GL_TEXTURE_2D, gl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
    }
} // namespace wrld::rsc
