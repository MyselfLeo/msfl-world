#include "components/Orbiter.hpp"
#include "components/PointLight.hpp"


#include <format>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>


#include "wrld/Logs.hpp"
#include "wrld/resources/Program.hpp"
#include "wrld/World.hpp"
#include "wrld/components/Camera.hpp"
#include "wrld/components/DirectionalLight.hpp"
#include "wrld/components/FPSControl.hpp"
#include "wrld/components/StaticModel.hpp"
#include "wrld/components/Transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "resources/WindowViewport.hpp"
#include "wrld/systems/RendererSystem.hpp"

using namespace wrld;

static std::shared_ptr<WindowViewport> window_viewport;

void window_resize_callback(GLFWwindow *window, const int width, const int height) {
    glViewport(0, 0, width, height);
    window_viewport->set_size(width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {}

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                            const char *message, const void *userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
        default:
            __builtin_unreachable();
    }
    std::cout << std::endl;

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
        default:
            __builtin_unreachable();
    }
    std::cout << std::endl;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
        default:
            __builtin_unreachable();
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

GLFWwindow *init_gl(const int width, const int height) {
    wrldInfo("Initialising OpenGL context");
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    wrldInfo("Creating window");
    GLFWwindow *window = glfwCreateWindow(width, height, "msflWorld Engine", nullptr, nullptr);

    if (window == nullptr) {
        const char *error = nullptr;
        int code = glfwGetError(&error);
        throw std::runtime_error(std::format("Failed to create GLFW window with error code {:0X}: {}", code, error));
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

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
    style.ScaleAllSizes(main_scale); // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
                                     // changing this requires resetting Style + calling this again)
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

Mesh generate_world_grid() {
    constexpr int NB_LINES = 10;
    constexpr float SPACING = 1; // Space between each lines

    // todo: make it not lighted
    auto vertex = Vertex({0, 0, 0}, {1, 0, 0}, {0, 0}, {0.7, 0.7, 0.7});

    float offset = SPACING * (NB_LINES / 2.0);
    if constexpr (NB_LINES % 2 == 0) {
        offset -= SPACING / 2.0;
    }

    // Create vertices
    std::vector<Vertex> vertices;
    std::vector<unsigned> elements;
    vertices.reserve(NB_LINES * 4);
    elements.reserve(NB_LINES * 4);

    // X direction
    for (int i = 0; i < NB_LINES; i++) {
        vertex.position = {-offset + SPACING * static_cast<float>(i), 0, offset};
        vertices.push_back(vertex);
        vertex.position = {-offset + SPACING * static_cast<float>(i), 0, -offset};
        vertices.push_back(vertex);
    }

    // Y direction
    for (int i = 0; i < NB_LINES; i++) {
        vertex.position = {offset, 0, -offset + SPACING * static_cast<float>(i)};
        vertices.push_back(vertex);
        vertex.position = {-offset, 0, -offset + SPACING * static_cast<float>(i)};
        vertices.push_back(vertex);
    }

    for (int i = 0; i < NB_LINES * 4; i++) {
        elements.push_back(i);
    }

    Mesh res{std::make_shared<Material>(), vertices, elements};
    res.set_gl_primitive_type(GL_LINES);
    res.update();

    return res;
}

Mesh generate_axis(float axis_length) {
    // todo: make it not lighted

    std::vector<Vertex> vertices;
    std::vector<unsigned> elements;
    vertices.reserve(6); // 3 lines so 6 vertices
    elements.reserve(6);

    // X => R
    vertices.push_back(Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));
    vertices.push_back(Vertex({axis_length, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));

    // Y => G
    vertices.push_back(Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));
    vertices.push_back(Vertex({0, axis_length, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));

    // Z => B
    vertices.push_back(Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 0, 1}));
    vertices.push_back(Vertex({0, 0, axis_length}, {1, 1, 1}, {0, 0}, {0, 0, 1}));

    for (int i = 0; i < 6; i++) {
        elements.push_back(i);
    }

    Mesh res{std::make_shared<Material>(), vertices, elements};
    res.set_gl_primitive_type(GL_LINES);
    res.update();

    return res;
}

int main() {
    GLFWwindow *window = init_gl(800, 600);

    World world;

    window_viewport = std::make_shared<WindowViewport>(window);
    glfwSetWindowSizeCallback(window, window_resize_callback);

    wrldInfo("Initialising systems");
    RendererSystem renderer{world, window};

    wrldInfo("Loading model");
    Model model("data/models/queen/queen.off");
    Model grid_model(generate_world_grid());
    Model axis_model(generate_axis(1));


    wrldInfo("Creating entities");
    const EntityID model_entity = world.create_entity();
    world.attach_component<cpt::StaticModel>(model_entity, model);
    world.attach_component<cpt::Transform>(model_entity);

    const EntityID world_grid = world.create_entity();
    world.attach_component<cpt::StaticModel>(world_grid, grid_model);

    const EntityID axis = world.create_entity();
    world.attach_component<cpt::StaticModel>(axis, axis_model);


    const EntityID camera_entity = world.create_entity();
    auto camera = world.attach_component<cpt::Camera>(camera_entity, 45, window_viewport);
    world.attach_component<cpt::Transform>(camera_entity);
    auto orbiter = world.attach_component<cpt::Orbiter>(camera_entity, model_entity, 2);
    orbiter->set_offset({0, 0, 0});

    auto env = world.attach_component<cpt::Environment>(camera_entity);
    env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 1.0, 1.0}, 0.0});

    const EntityID dir_light = world.create_entity();
    world.attach_component<cpt::DirectionalLight>(dir_light, glm::vec3{1.0, 1.0, 1.0}, 1.0);
    world.attach_component<cpt::Transform>(dir_light)->look_towards({-1, 0, -1}, {0, 1, 0});
    bool show_demo_window = true;
    bool reloading = false;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !reloading) {
            reloading = true;
            camera->get_program()->reload();
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            reloading = false;
        }
        glClearColor(0.1f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Rotate backpack
        const auto ROTATION_RATE = glm::quat(glm::vec3{0, 0.005, 0});

        const auto backpack_transform = world.get_component<cpt::Transform>(model_entity);
        auto curr_rotation = backpack_transform->get_rotation();
        backpack_transform->set_rotation(ROTATION_RATE * curr_rotation);

        float camera_speed = 1.5;

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            orbiter->set_vert_angle(orbiter->get_vert_angle() + camera_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            orbiter->set_vert_angle(orbiter->get_vert_angle() - camera_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            orbiter->set_hor_angle(orbiter->get_hor_angle() + camera_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            orbiter->set_hor_angle(orbiter->get_hor_angle() - camera_speed);
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            orbiter->set_distance(orbiter->get_distance() * 0.95);
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            orbiter->set_distance(orbiter->get_distance() / 0.95);
        }

        orbiter->update();

        renderer.exec();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
