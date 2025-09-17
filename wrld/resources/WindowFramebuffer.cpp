//
// Created by leo on 9/4/25.
//

#include "WindowFramebuffer.hpp"

#include "logs.hpp"

#include <iostream>

namespace wrld::rsc {
    WindowFramebuffer::WindowFramebuffer(GLFWwindow *window) : window(window) { // NOLINT(*-pro-type-member-init)
        glfwGetWindowSize(window, &width, &height);
    }

    int WindowFramebuffer::get_width() const { return width; }

    int WindowFramebuffer::get_height() const { return height; }

    void WindowFramebuffer::use() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, this->width, this->height);
    }

    GLuint WindowFramebuffer::get_fbo() const { return 0; }

    void WindowFramebuffer::set_size(const unsigned width, const unsigned height) {
        // glfwSetWindowSize(window, width, heigth);
        this->width = width;
        this->height = height;
    }
} // namespace wrld::rsc
