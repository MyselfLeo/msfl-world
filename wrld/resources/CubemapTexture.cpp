//
// Created by leo on 8/22/25.
//

#include "CubemapTexture.hpp"

#include "logs.hpp"
#include "stb_image.hpp"

#include <format>
#include <iostream>
#include <ranges>

namespace wrld::rsc {
    CubemapTexture::CubemapTexture(const std::vector<std::string> &cubemap_paths) : gl_texture(0) {
        stbi_set_flip_vertically_on_load(false);

        // Filtering for cubemap
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &gl_texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl_texture);

        // Load each faces into the glTexture
        int width, height, nb_channels;
        for (const auto [i, text_path]: std::ranges::views::enumerate(cubemap_paths)) {
            unsigned char *data = stbi_load(text_path.c_str(), &width, &height, &nb_channels, 0);

            if (data == nullptr) {
                stbi_image_free(data);
                throw std::runtime_error(std::format("Error while loading texture {}", text_path));
            }

            if (nb_channels != 3) {
                stbi_image_free(data);
                throw std::runtime_error("Only RGB images are supported for now");
            }

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);

            stbi_image_free(data);
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    CubemapTexture::CubemapTexture(CubemapTexture &&other) noexcept : gl_texture(other.gl_texture) {
        other.gl_texture = 0;
    }

    CubemapTexture &CubemapTexture::operator=(CubemapTexture &&other) noexcept {
        if (gl_texture != 0)
            glDeleteTextures(1, &gl_texture);

        gl_texture = other.gl_texture;
        other.gl_texture = 0;

        return *this;
    }

    void CubemapTexture::use(const unsigned unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl_texture);
    }

    CubemapTexture::~CubemapTexture() { glDeleteTextures(1, &gl_texture); }
} // namespace wrld::rsc
