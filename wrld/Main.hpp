//
// Created by leo on 9/11/25.
//

#ifndef MAIN_HPP
#define MAIN_HPP
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include "App.hpp"
#include "World.hpp"
#include "resources/WindowViewport.hpp"

namespace wrld {

    class Main {
    public:
        static void run(App &app, unsigned width, unsigned height);
        static void exit();

        static GLFWwindow *get_window();
        static std::shared_ptr<rsc::WindowViewport> get_window_viewport();

    private:
        static World world;
        static GLFWwindow *window;
        static std::shared_ptr<rsc::WindowViewport> window_viewport;

        static bool should_close;

        // Deltatime computing
        static double last_frame;

        static GLFWwindow *init_gl(int width, int height);
        static void window_resize_callback(GLFWwindow *window, int width, int height);
        static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                                           const char *message, const void *userParam);
    };

} // namespace wrld

#endif // MAIN_HPP
