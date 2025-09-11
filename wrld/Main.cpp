//
// Created by leo on 9/11/25.
//
#include <iostream>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Main.hpp"

#include "logs.hpp"
#include "systems/RendererSystem.hpp"

namespace wrld {
    // Default Main values
    World Main::world;
    GLFWwindow *Main::window = nullptr;
    std::shared_ptr<rsc::WindowViewport> Main::window_viewport = nullptr;
    bool Main::should_close = false;
    double Main::last_frame = 0;

    void Main::run(App &app, const unsigned width, const unsigned height) {
        window = init_gl(width, height);
        window_viewport = std::make_shared<rsc::WindowViewport>(window);
        glfwSetWindowSizeCallback(window, window_resize_callback);

        world = World();

        // Create systems
        wrldInfo("Initialising systems");
        RendererSystem renderer{world, window};

        should_close = false;
        app.init(world);

        while (!should_close) {
            // todo: move this to the camera
            glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // Compute deltatime
            const double current_frame = glfwGetTime();
            const double deltatime = current_frame - last_frame; // in seconds
            last_frame = current_frame;

            // Update user application
            app.update(world, deltatime);

            // Execute systems
            renderer.exec();

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
        app.exit(world);
    }

    void Main::exit() { should_close = true; }

    GLFWwindow *Main::get_window() { return window; }

    std::shared_ptr<rsc::WindowViewport> Main::get_window_viewport() { return window_viewport; }

    GLFWwindow *Main::init_gl(const int width, const int height) {
        wrldInfo("Initialising OpenGL context");
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

        const float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

        wrldInfo("Creating window");
        GLFWwindow *window = glfwCreateWindow(width, height, "msflWorld Engine", nullptr, nullptr);

        if (window == nullptr) {
            const char *error = nullptr;
            int code = glfwGetError(&error);
            throw std::runtime_error(
                    std::format("Failed to create GLFW window with error code {:0X}: {}", code, error));
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
        style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style
                                         // scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi = main_scale; // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this
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
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
        return window;
    }

    void Main::window_resize_callback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
        // window_viewport->set_size(width, height);
    }

    void APIENTRY Main::glDebugOutput(const GLenum source, GLenum type, const unsigned id, GLenum severity,
                                      GLsizei length, const char *message, const void *userParam) {
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

} // namespace wrld
