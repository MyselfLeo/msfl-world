//
// Created by leo on 1/6/26.
//

#include <wrld/resources/TAAFramebuffer.hpp>

namespace wrld::rsc {
    TAAFramebuffer::TAAFramebuffer(std::string name, World &world) : DeferredFramebuffer(std::move(name), world),
                                                                     history_texture(0) {
    }

    GLuint TAAFramebuffer::get_history_texture() const {
        return history_texture;
    }

    void TAAFramebuffer::recreate() {
        // Generate new framebuffer & textures
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // position texture
        glGenTextures(1, &position_texture);
        glBindTexture(GL_TEXTURE_2D, position_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // normal texture
        glGenTextures(1, &normal_texture);
        glBindTexture(GL_TEXTURE_2D, normal_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8_SNORM, width, height, 0, GL_RGB, GL_FLOAT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // diffuse texture
        glGenTextures(1, &diffuse_texture);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT,
                     nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // lighting mode texture
        glGenTextures(1, &do_lighting_texture);
        glBindTexture(GL_TEXTURE_2D, do_lighting_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, width, height, 0, GL_RED_INTEGER,
                     GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glGenTextures(1, &history_texture);
        glBindTexture(GL_TEXTURE_2D, history_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // depth texture
        glGenTextures(1, &depth_texture);
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
                     GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach textures to framebuffer
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, position_texture,
                             0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normal_texture,
                             0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, diffuse_texture,
                             0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,
                             do_lighting_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4,
                             history_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void TAAFramebuffer::use() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, this->width, this->height);

        constexpr GLenum buffers[] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, buffers);
    }
}
