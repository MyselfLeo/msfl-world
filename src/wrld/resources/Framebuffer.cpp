//
// Created by leo on 9/12/25.
//

#include <wrld/resources/Framebuffer.hpp>

#include <format>

namespace wrld::rsc {
    Framebuffer::Framebuffer(std::string name, World &world /*, Rc<Resource> *rc*/) :
        Resource(std::move(name), world /*, rc*/), fbo(0), width(0), height(0) {}

    Framebuffer::~Framebuffer() {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }

    GLuint Framebuffer::get_fbo() const { return fbo; }

    Framebuffer &Framebuffer::set_size(const unsigned width, const unsigned height) {
        this->width = width;
        this->height = height;
        return *this;
    }

    Framebuffer &Framebuffer::set_nb_color_attachments(const unsigned nb) {
        static constexpr int max_color_attachments = GL_MAX_COLOR_ATTACHMENTS - GL_COLOR_ATTACHMENT0;
        if (nb > max_color_attachments) {
            throw std::runtime_error(std::format("Maximum of color attachments is {}", max_color_attachments));
        }
        if (nb == 0) {
            throw std::runtime_error("A framebuffer needs to have at least one color attachment");
        }
        this->nb_color_attachments = nb;
        return *this;
    }

    unsigned Framebuffer::get_width() const { return this->width; }

    unsigned Framebuffer::get_height() const { return this->height; }

    void Framebuffer::use() const {
        glViewport(0, 0, this->width, this->height);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    }

    // todo: this function is not generalised.
    // we need to be able to specify each texture type
    // possibly just give the texture directly
    void Framebuffer::recreate() {
        // Clear previously defined OpenGL objects
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
        if (depth_texture != 0) {
            glDeleteTextures(1, &depth_texture);
        }
        glDeleteTextures(color_textures.size(), color_textures.data());
        color_textures.clear();

        // todo: use rcs::Texture instead

        // Generate new framebuffer & textures
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        // color textures
        for (int i = 0; i < nb_color_attachments; i++) {
            GLuint color_texture;
            glGenTextures(1, &color_texture);
            glBindTexture(GL_TEXTURE_2D, color_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            //? glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
            glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color_texture, 0);

            color_textures.push_back(color_texture);
        }

        // depth texture
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                     nullptr);


        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
} // namespace wrld::rsc
