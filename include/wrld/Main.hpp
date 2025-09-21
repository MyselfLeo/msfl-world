//
// Created by leo on 9/11/25.
//

#ifndef MAIN_HPP
#define MAIN_HPP
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include <wrld/App.hpp>
#include <wrld/World.hpp>
#include <wrld/resources/WindowFramebuffer.hpp>
#include <wrld/systems/RendererSystem.hpp>

namespace wrld {

    enum RendererType {
        FORWARD_RENDERER,
        DEFERRED_RENDERER,
    };

    class Main {
    public:
        static void run(App &app, unsigned width, unsigned height);
        static void exit();

        static GLFWwindow *get_window();
        static std::shared_ptr<rsc::WindowFramebuffer> get_window_viewport();

        static void set_renderer_type(RendererType _renderer_type);

    private:
        static World world;
        static GLFWwindow *window;
        static std::shared_ptr<rsc::WindowFramebuffer> window_viewport;

        static RendererType renderer_type;

        static bool should_close;

        // Deltatime computing
        static double last_frame;

        static std::unique_ptr<RendererSystem> get_renderer();

        static GLFWwindow *init_gl(int width, int height);
        static void window_resize_callback(GLFWwindow *window, int width, int height);
        static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                                           const char *message, const void *userParam);
    };

} // namespace wrld

#endif // MAIN_HPP
