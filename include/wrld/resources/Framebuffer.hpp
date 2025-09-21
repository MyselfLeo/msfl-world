//
// Created by leo on 9/12/25.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP
#include <wrld/resources/Resource.hpp>
#include "glad/glad.h"

#include <vector>

namespace wrld::rsc {
    class Framebuffer : public Resource {
    public:
        Framebuffer(std::string name, World &world /*, Rc<Resource> *rc*/);

        ~Framebuffer() override;

        [[nodiscard]] GLuint get_fbo() const;

        Framebuffer &set_size(unsigned width, unsigned height);

        Framebuffer &set_nb_color_attachments(unsigned nb);

        [[nodiscard]] unsigned get_width() const;
        [[nodiscard]] unsigned get_height() const;

        std::string get_type() const override { return "Viewport"; }

        void use() const;

        void recreate();

    private:
        GLuint fbo;

        unsigned nb_color_attachments = 1;
        std::vector<GLuint> color_textures;
        GLuint depth_texture;
        // GLuint specular_texture;

        unsigned width, height;
    };

} // namespace wrld::rsc

#endif // VIEWPORT_HPP
