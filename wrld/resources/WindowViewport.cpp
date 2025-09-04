//
// Created by leo on 9/4/25.
//

#include "WindowViewport.hpp"

namespace wrld {
    WindowViewport::WindowViewport(GLFWwindow *window) : window(window) { // NOLINT(*-pro-type-member-init)
        glfwGetWindowSize(window, &width, &height);
    }

    int WindowViewport::get_width() const { return width; }

    int WindowViewport::get_height() const { return height; }

    void WindowViewport::set_size(const unsigned width, const unsigned height) {
        // glfwSetWindowSize(window, width, heigth);
        this->width = width;
        this->height = height;
    }
} // namespace wrld
