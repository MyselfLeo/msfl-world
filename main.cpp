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
#include "wrld/systems/RendererSystem.hpp"

using namespace wrld;

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *window, const int width, const int height) {
    glViewport(0, 0, width, height);
}

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
    // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());

    wrldInfo("Creating window");
    GLFWwindow *window = glfwCreateWindow(width, height, "msflWorld Engine", nullptr, nullptr);

    if (window == nullptr) {
        const char *error = nullptr;
        int code = glfwGetError(&error);
        throw std::runtime_error(std::format("Failed to create GLFW window with error code {:0X}: {}", code, error));
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

int main() {
    GLFWwindow *window = init_gl(800, 600);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    World world;

    wrldInfo("Initialising systems");
    RendererSystem renderer{world, window};

    wrldInfo("Loading model");
    Model backpack_model("data/models/backpack/backpack.obj");
    Model myshape_model("data/models/myshape/myshape.obj");
    Model cube_model("data/models/cube/cube.obj");

    wrldInfo("Loading skybox");
    auto skybox = std::make_shared<CubemapTexture>(std::vector<std::string>{
            "data/textures/lake_cm/right.jpg", "data/textures/lake_cm/left.jpg", "data/textures/lake_cm/top.jpg",
            "data/textures/lake_cm/bottom.jpg", "data/textures/lake_cm/front.jpg", "data/textures/lake_cm/back.jpg"});

    wrldInfo("Creating entities");
    const EntityID backpack = world.create_entity();
    world.attach_component<cpt::StaticModel>(backpack, backpack_model);
    world.attach_component<cpt::Transform>(backpack, glm::vec3{4.0, 0.0, 0.0});

    const EntityID myshape = world.create_entity();
    world.attach_component<cpt::StaticModel>(myshape, myshape_model);
    world.attach_component<cpt::Transform>(myshape, glm::vec3{-4.0, 0.0, 0.0});

    const EntityID ground = world.create_entity();
    world.attach_component<cpt::StaticModel>(ground, cube_model);
    world.attach_component<cpt::Transform>(ground, glm::vec3{0, -5, 0}, glm::quat{1, 0, 0, 0},
                                           glm::vec3(100, 0.2, 100));

    const EntityID camera = world.create_entity();
    world.attach_component<cpt::Camera>(camera, 45);
    auto camera_transform = world.attach_component<cpt::Transform>(camera, glm::vec3{0.0, 0.0, 8.0});
    const auto move = world.attach_component<cpt::FPSControl>(camera);
    auto env = world.attach_component<cpt::Environment>(camera);
    env->set_cubemap(skybox);
    env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0, 1.0, 1.0}, 0.4});

    const EntityID dir_light = world.create_entity();
    world.attach_component<cpt::DirectionalLight>(dir_light, glm::vec3{1.0, 1.0, 1.0}, 0.4);
    const auto dir_light_transform = world.attach_component<cpt::Transform>(dir_light);
    world.attach_component<cpt::StaticModel>(dir_light, cube_model);

    dir_light_transform->set_scale(glm::vec3{0.1, 0.1, 0.1});
    dir_light_transform->set_position(glm::vec3{0, 1, 0});
    dir_light_transform->look_towards(glm::vec3{0, 0, -1}, glm::vec3{0, 1, 0});

    /*const EntityID light1 = world.create_entity();
    world.attach_component<cpt::PointLight>(light1, glm::vec3{0.0, 1.0, 0.0}, 30.0);
    const auto light_transform1 = world.attach_component<cpt::Transform>(light1);
    world.attach_component<cpt::StaticModel>(light1, cube_model);
    light_transform1->set_scale(glm::vec3{0.1, 0.1, 0.1});
    const EntityID light2 = world.create_entity();
    world.attach_component<cpt::PointLight>(light2, glm::vec3{1.0, 0.0, 0.0}, 30.0);
    const auto light_transform2 = world.attach_component<cpt::Transform>(light2);
    world.attach_component<cpt::StaticModel>(light2, cube_model);
    light_transform2->set_scale(glm::vec3{0.1, 0.1, 0.1});*/

    const EntityID light3 = world.create_entity();
    world.attach_component<cpt::PointLight>(light3, glm::vec3{0.0, 0.0, 1.0}, 5.0);
    const auto light_transform3 = world.attach_component<cpt::Transform>(light3);
    world.attach_component<cpt::StaticModel>(light3, cube_model);
    light_transform3->set_scale(glm::vec3{0.1, 0.1, 0.1});

    bool show_demo_window = true;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        const double time = glfwGetTime();
        processInput(window);

        // camera_transform->set_towards({0, 0, 0});
        move->update(window);

        // Rotate backpack
        const auto ROTATION_RATE = glm::quat(glm::vec3{0, 0.005, 0});

        /*const auto backpack_transform = world.get_component<cpt::Transform>(backpack);
        auto curr_rotation = backpack_transform->get_rotation();
        backpack_transform->set_rotation(ROTATION_RATE * curr_rotation);

        const auto myshape_transform = world.get_component<cpt::Transform>(myshape);
        curr_rotation = myshape_transform->get_rotation();
        myshape_transform->set_rotation(ROTATION_RATE * curr_rotation);*/
        //
        auto curr_rotation = dir_light_transform->get_rotation();
        dir_light_transform->set_rotation(ROTATION_RATE * curr_rotation);

        // light_transform1->set_position({0, sin(time) * 4, 0});
        // light_transform2->set_position({0, sin(time + M_PI) * 4, 0});
        light_transform3->set_position({0, sin(time + M_PI / 2) * 4, 0});


        renderer.exec();

        // Start the Dear ImGui frame
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
