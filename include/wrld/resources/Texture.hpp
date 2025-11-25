//
// Created by leo on 8/8/25.
//

#pragma once

#include <wrld/resources/Resource.hpp>
#include "assimp/material.h"


#include <string>

namespace wrld::rsc {
    class Texture final : public Resource {
    public:
        explicit Texture(const std::string &name, World &world);

        Texture &from_file(const std::string &texture_path, aiTextureType type,
                           bool flip_textures = false);

        /// Returns a default texture.
        Texture &from_default();

        Texture(Texture &other) = delete;
        Texture(Texture &&other) = delete;
        Texture &operator=(Texture &other) = delete;
        Texture &operator=(Texture &&other) = delete;

        void use(unsigned unit = 0) const;

        ~Texture() override;

        std::string get_type() const override { return "Texture"; }

    private:
        void update_texture(int width, int height, int nb_channels, unsigned char *data);

        unsigned gl_texture = 0;

        // Using Assimp enum for now, it's good enough
        aiTextureType type = aiTextureType_DIFFUSE;
    };
} // namespace wrld::rsc
