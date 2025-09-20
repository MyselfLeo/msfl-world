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
        explicit Texture(const std::string &name, World &world /*, Rc<Resource> *rc*/);

        Texture &set_texture(const std::string &texture_path, aiTextureType type, bool flip_textures = false);

        Texture(Texture &other) = delete;
        Texture(Texture &&other) = delete;
        Texture &operator=(Texture &other) = delete;
        Texture &operator=(Texture &&other) = delete;

        void use(unsigned unit = 0) const;

        ~Texture() override;

        std::string get_type() const override { return "Texture"; }

    private:
        GLuint gl_texture = 0;
        std::string path = "data/textures/default.png";
        bool flip_textures = true;

        void reload();

    public:
        void load_default_resources() override;

    private:
        // Using Assimp enum for now, it's good enough
        aiTextureType type = aiTextureType_DIFFUSE;
    };
} // namespace wrld::rsc

#endif // TEXTURE_HPP
