//
// Created by leo on 9/11/25.
//
#include <iostream>

#include "glad/glad.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <wrld/Main.hpp>

#include <wrld/logs.hpp>
#include <wrld/systems/DeferredRendererSystem.hpp>
#include <wrld/systems/RendererSystem.hpp>

#include <utility>

namespace wrld {
    // Default Main values
    World Main::world;
    GLFWwindow *Main::window = nullptr;
    std::shared_ptr<rsc::WindowFramebuffer> Main::window_viewport = nullptr;
    bool Main::should_close = false;
    double Main::last_frame = 0;
    double Main::delta_time = 0;
    std::string Main::window_title = "Unnamed";
    glm::vec3 Main::clear_color = {0.08, 0.08, 0.08};
    RendererType Main::renderer_type = RendererType::ForwardRenderer;
    std::unordered_map<std::string, std::string> Main::statistics = {};

    void Main::run(App &app, const unsigned width, const unsigned height) {
        wrldInfo("msfl-world");

        switch (get_platform()) {
            case Platform::Clang: {
                wrldInfo("Platform: CLANG");
            } break;
            case Platform::GCC: {
                wrldInfo("Platform: GCC");
            } break;
            case Platform::MSVC: {
                wrldInfo("Platform: MSVC");
            } break;
        }

        init_gl(width, height);
        window_viewport = std::make_shared<rsc::WindowFramebuffer>(window);
        glfwSetWindowSizeCallback(window, window_resize_callback);

        world = World();

        // Create systems
        wrldInfo("Initialising systems");

        std::unique_ptr<RendererSystem> renderer;

        // todo: make RendererSystem abstract, implement
        // ForwardRendererSystem (which replace RendererSystem) and
        // NoRendererSystem (doesn't render anything).
        if (renderer_type != RendererType::NoRenderer) {
            renderer = get_renderer();
        }

        wrldInfo("Initializing app");
        app.init(world);

        wrldInfo("Starting main loop");
        while (!should_close) {
            // Update user application
            app.update(world, delta_time);

            // Compute deltatime
            const double current_frame = glfwGetTime();
            delta_time = current_frame - last_frame; // in seconds
            last_frame = current_frame;

            if (renderer_type != RendererType::NoRenderer) {
                // todo: move this to the camera
                glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                renderer->exec(delta_time);

                // Render UI using ImGUI
                {
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    app.ui(world);

                    ImGui::Render();
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                }

                // Prepare next frame
                glfwSwapBuffers(window);
                glfwPollEvents();

                if (glfwWindowShouldClose(window)) {
                    should_close = true;
                }
            }
        }

        wrldInfo("Exiting app");
        app.exit(world);
    }

    void Main::exit() { should_close = true; }

    GLFWwindow *Main::get_window() { return window; }

    std::shared_ptr<rsc::WindowFramebuffer> Main::get_window_viewport() {
        return window_viewport;
    }

    double Main::get_time() { return glfwGetTime(); }

    double Main::get_delta_time() { return delta_time; }

    void Main::set_renderer_type(const RendererType _renderer_type) {
        renderer_type = _renderer_type;
    }

    void Main::set_clear_color(const glm::vec3 &color) { clear_color = color; }

    void Main::set_window_title(const std::string &title) {
        window_title = title;
        if (window != nullptr)
            update_window_title();
    }

    void Main::set_statistic(const std::string &stat_name, const std::string &value) {
        statistics.insert_or_assign(stat_name, value);
    }

    const std::unordered_map<std::string, std::string> &Main::get_statistics() {
        return statistics;
    }

    std::unique_ptr<RendererSystem> Main::get_renderer() {
        switch (renderer_type) {
            case RendererType::ForwardRenderer:
                wrldInfo("Using forward renderer");
                return std::make_unique<RendererSystem>(world, window);
            case RendererType::DeferredRenderer:
                wrldInfo("Using deferred renderer");
                return std::make_unique<DeferredRendererSystem>(world, window);
            default:
                std::unreachable();
        }
    }

    void Main::update_window_title() {
        glfwSetWindowTitle(window, std::format("{} :: msfl-world", window_title).c_str());
    }

    GLFWwindow *Main::init_gl(const int width, const int height) {
        wrldInfo("Initialising OpenGL context");
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

        const float main_scale =
                ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

        wrldInfo("Creating window");
        window = glfwCreateWindow(width, height,
                                  std::format("{} :: msfl-world", window_title).c_str(),
                                  nullptr, nullptr);

        if (window == nullptr) {
            const char *error = nullptr;
            int code = glfwGetError(&error);
            throw std::runtime_error(
                    std::format("Failed to create GLFW window with error code {:0X}: {}",
                                code, error));
        }

        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup scaling
        ImGuiStyle &style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a
                                         // solution for dynamic style
        // scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale; // Set initial font scale. (using
                                         // io.ConfigDpiScaleFonts=true makes this
        // unnecessary. We leave both here for documentation purpose)

        ImGui_ImplOpenGL3_Init();
        ImGui_ImplGlfw_InitForOpenGL(window, true);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glViewport(0, 0, width, height);

        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            wrldInfo("Enabling OpenGL debug");
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                                  GL_TRUE);
        }
        return window;
    }

    void Main::window_resize_callback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        // window_viewport->set_size(width, height);
    }

#ifndef NDEBUG
    void APIENTRY Main::glDebugOutput(const GLenum source, GLenum type, const unsigned id,
                                      GLenum severity, GLsizei length,
                                      const char *message, const void *userParam) {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;

        switch (source) {
            case GL_DEBUG_SOURCE_API:
                std::cout << "[wrld:gl:api] ";
                break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                std::cout << "[wrld:gl:window] ";
                break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:
                std::cout << "[wrld:gl:shader] ";
                break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:
                std::cout << "[wrld:gl:third_part] ";
                break;
            case GL_DEBUG_SOURCE_APPLICATION:
                std::cout << "[wrld:gl:app] ";
                break;
            case GL_DEBUG_SOURCE_OTHER:
                std::cout << "[wrld:gl:other] ";
                break;
            default:
                std::cout << "[wrld:gl:unknown] ";
                break;
        }

        std::cout << message << std::endl;
    }
#else
    void APIENTRY Main::glDebugOutput(const GLenum source, GLenum type, const unsigned id,
                                      GLenum severity, GLsizei length,
                                      const char *message, const void *userParam) {}
#endif

} // namespace wrld
