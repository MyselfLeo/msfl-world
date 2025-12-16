//
// Created by leo on 9/11/25.
//

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>

#include <wrld/App.hpp>
#include <wrld/World.hpp>
#include <wrld/resources/WindowFramebuffer.hpp>

namespace wrld {
    enum class RendererType {
        ForwardRenderer,
        DeferredRenderer,
        TAARenderer,
        NoRenderer,
    };

    enum class Platform {
        MSVC,
        GCC,
        Clang,
    };

    class Main {
    public:
        static void run(App &app, unsigned width, unsigned height);

        static void exit();

        /// Return the window
        static GLFWwindow *get_window();

        /// Return the window frame-buffer
        static std::shared_ptr<rsc::WindowFramebuffer> get_window_viewport();

        /// Return the time elapsed since the start of the program
        static double get_time();

        /// Return the current delta_time
        static double get_delta_time();

        static void set_renderer_type(RendererType _renderer_type);

        static void set_clear_color(const glm::vec3 &color);

        static void set_window_title(const std::string &title);

        /// Defines a statistic that may be accessed using Main::get_statistics
        static void set_statistic(const std::string &stat_name, const std::string &value);

        /// Returns all statistics.
        static const std::unordered_map<std::string, std::string> &get_statistics();

        /// Return the platform used to compile the program.
        /// Can be used at compile-time (constexpr).
        static constexpr Platform get_platform() {
            #if defined(__clang__)
            return Platform::Clang;
            #elif defined(__GNUC__) || defined(__GNUG__)
            return Platform::GCC;
            #elif defined(_MSC_VER)
            return Platform::MSVC;
            #endif
        }

    private:
        static World world;
        static GLFWwindow *window;
        static std::shared_ptr<rsc::WindowFramebuffer> window_viewport;

        static RendererType renderer_type;

        static bool should_close;

        // Deltatime computing
        static double last_frame;
        static double delta_time;

        // todo: move to Camera3D
        static glm::vec3 clear_color;

        static std::string window_title;

        static std::unordered_map<std::string, std::string> statistics;

        static void update_window_title();

        static GLFWwindow *init_gl(int width, int height);

        static void window_resize_callback(GLFWwindow *window, int width, int height);

        static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id,
                                           GLenum severity, GLsizei length,
                                           const char *message, const void *userParam);
    };
} // namespace wrld
