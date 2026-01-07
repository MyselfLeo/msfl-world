//
// Created by leo on 9/12/25.
//

#pragma once

#include <wrld/resources/Resource.hpp>
#include "glad/glad.h"

#include <vector>

namespace wrld::rsc {
    class Framebuffer : public Resource {
    public:
        Framebuffer(std::string name, World &world);

        ~Framebuffer() override;

        [[nodiscard]] virtual GLuint get_fbo() const;

        virtual Framebuffer &set_size(unsigned width, unsigned height);

        Framebuffer &set_nb_color_attachments(unsigned nb);

        [[nodiscard]] virtual unsigned get_width() const;

        [[nodiscard]] virtual unsigned get_height() const;

        std::string get_type() const override { return "Viewport"; }

        virtual void use() const;

        virtual void recreate();

    private:
        GLuint fbo;

        unsigned nb_color_attachments = 1;
        std::vector<GLuint> color_textures;
        GLuint depth_texture;
        // GLuint specular_texture;

        unsigned width, height;
    };
} // namespace wrld::rsc
