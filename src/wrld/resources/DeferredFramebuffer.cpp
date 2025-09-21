//
// Created by leo on 9/12/25.
//

#include <wrld/resources/DeferredFramebuffer.hpp>
#include <wrld/resources/Framebuffer.hpp>

#include <format>

namespace wrld::rsc {
    DeferredFramebuffer::DeferredFramebuffer(std::string name, World &world /*, Rc<Resource> *rc*/) :
        Resource(std::move(name), world /*, rc*/), fbo(0), position_texture(0), normal_texture(0), diffuse_texture(0),
        depth_texture(0), width(0), height(0) {}

    DeferredFramebuffer::~DeferredFramebuffer() {
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
    }

    GLuint DeferredFramebuffer::get_fbo() const { return fbo; }

    DeferredFramebuffer &DeferredFramebuffer::set_size(const unsigned width, const unsigned height) {
        this->width = width;
        this->height = height;
        return *this;
    }

    unsigned DeferredFramebuffer::get_width() const { return this->width; }

    unsigned DeferredFramebuffer::get_height() const { return this->height; }

    void DeferredFramebuffer::use() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, this->width, this->height);
    }

    // todo: this function is not generalised.
    // we need to be able to specify each texture type
    // possibly just give the texture directly
    void DeferredFramebuffer::recreate() {
        // Clear previously defined OpenGL objects
        if (fbo != 0) {
            glDeleteFramebuffers(1, &fbo);
        }
        if (position_texture != 0) {
            glDeleteTextures(1, &depth_texture);
        }
        if (normal_texture != 0) {
            glDeleteTextures(1, &depth_texture);
        }
        if (diffuse_texture != 0) {
            glDeleteTextures(1, &depth_texture);
        }
        if (depth_texture != 0) {
            glDeleteTextures(1, &depth_texture);
        }

        // Generate new framebuffer & textures
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        // position texture
        glGenTextures(1, &position_texture);
        glBindTexture(GL_TEXTURE_2D, position_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // normal texture
        glGenTextures(1, &normal_texture);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // diffuse texture
        glGenTextures(1, &diffuse_texture);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // depth texture
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach textures to framebuffer
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, position_texture, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_texture, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, diffuse_texture, 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

        // Link program output to textures
        constexpr GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, buffers);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    GLuint DeferredFramebuffer::get_position_texture() const { return position_texture; }

    GLuint DeferredFramebuffer::get_normal_texture() const { return normal_texture; }

    GLuint DeferredFramebuffer::get_diffuse_texture() const { return diffuse_texture; }

    GLuint DeferredFramebuffer::get_depth_texture() const { return depth_texture; }
} // namespace wrld::rsc
