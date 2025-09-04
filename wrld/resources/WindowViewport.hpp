//
// Created by leo on 9/4/25.
//

#ifndef WINDOWVIEWPORT_HPP
#define WINDOWVIEWPORT_HPP
#include "../../lib/glfw-3.4/include/GLFW/glfw3.h"

namespace wrld {

    class WindowViewport {
    public:
        explicit WindowViewport(GLFWwindow *window);

        int get_width() const;
        int get_height() const;

        void set_size(unsigned width, unsigned height);

    private:
        int width, height;
        GLFWwindow *window;
    };

} // namespace wrld

#endif // WINDOWVIEWPORT_HPP
