#include <format>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>


#include "Camera.hpp"
#include "Logs.hpp"
#include "resources/Program.hpp"
#include "World.hpp"
#include "components/Camera.hpp"
#include "components/StaticModel.hpp"
#include "components/PointLight.hpp"
#include "components/Transform.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "resources/Texture.hpp"
#include "systems/RendererSystem.hpp"

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
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

GLFWwindow *init_gl(int width, int height) {
    wrldInfo("Initialising OpenGL context");
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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

    glEnable(GL_DEPTH_TEST);
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

    wrldInfo("Loading model");
    Model backpack_model("data/models/backpack/backpack.obj");
    Model myshape_model("data/models/myshape/myshape.obj");
    Model cube_model("data/models/cube/cube.obj");

    World world;

    wrldInfo("Creating entities");
    const EntityID backpack = world.create_entity();
    world.attach_component<cpt::StaticModel>(backpack, backpack_model);
    world.attach_component<cpt::Transform>(backpack, glm::vec3{2.0, 0.0, 0.0});

    const EntityID myshape = world.create_entity();
    world.attach_component<cpt::StaticModel>(myshape, myshape_model);
    world.attach_component<cpt::Transform>(myshape, glm::vec3{-2.0, 0.0, 0.0});

    const EntityID camera = world.create_entity();
    world.attach_component<cpt::Camera>(camera, 45);
    world.attach_component<cpt::Transform>(camera, glm::vec3{0.0, 0.0, -8.0});

    const EntityID light = world.create_entity();
    world.attach_component<cpt::PointLight>(light, glm::vec3{1.0, 1.0, 1.0}, 1.0);
    const auto light_transform = world.attach_component<cpt::Transform>(light);
    world.attach_component<cpt::StaticModel>(light, cube_model);
    light_transform->set_scale(glm::vec3{0.1, 0.1, 0.1});

    RendererSystem renderer{world, window};

    while (!glfwWindowShouldClose(window)) {
        const double time = glfwGetTime();
        processInput(window);

        // Rotate backpack
        const auto ROTATION_RATE = glm::quat(glm::vec3{0, 0.01, 0});

        const auto backpack_transform = world.get_component<cpt::Transform>(backpack);
        auto curr_rotation = backpack_transform->get_rotation();
        backpack_transform->set_rotation(ROTATION_RATE * curr_rotation);

        const auto myshape_transform = world.get_component<cpt::Transform>(myshape);
        curr_rotation = myshape_transform->get_rotation();
        myshape_transform->set_rotation(ROTATION_RATE * curr_rotation);

        light_transform->set_position({0, sin(time) * 4, 0});


        renderer.exec();

        glfwPollEvents();
    }

    return 0;
}
