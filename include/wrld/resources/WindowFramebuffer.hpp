//
// Created by leo on 9/4/25.
//

#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace wrld::rsc {

    class WindowFramebuffer {
    public:
        explicit WindowFramebuffer(GLFWwindow *window);

        int get_width() const;
        int get_height() const;

        void use() const;

        GLuint get_fbo() const;

        void set_size(unsigned width, unsigned height);

    private:
        int width, height;
        GLFWwindow *window;
    };

} // namespace wrld::rsc
