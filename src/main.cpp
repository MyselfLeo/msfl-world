#include <format>
#include <fstream>
#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLint load_shader(const std::string &shader_file, GLint type) {
    std::ifstream file(shader_file);

    if (!file.is_open()) {
        std::cerr << std::format("Failed to load shader {}", shader_file) << std::endl;
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string shader_source = buffer.str();
    const char *shader_source_str = shader_source.c_str();

    file.close();

    GLint vertex_shader = glCreateShader(type);

    glShaderSource(vertex_shader, 1, &shader_source_str, nullptr);
    glCompileShader(vertex_shader);

    // Check compilation error
    int success;
    char infoLog[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, nullptr, infoLog);
        std::cerr << std::format("Failed to compile shader {}: {}", shader_file, infoLog) << std::endl;
        std::exit(1);
    }

    return vertex_shader;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLuint create_object() {
    static constexpr float vertices[] = {
        0.5f, 0.5f, 0.0f, // top right
        0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f // top left
    };

    static constexpr unsigned indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3 // second triangle
    };
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    return vao;
}

void render() {
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Ma Window", nullptr, nullptr);

    if (window == nullptr) {
        const char *error = nullptr;
        int code = glfwGetError(&error);
        std::cerr << std::format("Failed to create GLFW window with error code {:0X}: {}", code, error) << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLint vertex_shader = load_shader("src/shaders/vertex.glsl", GL_VERTEX_SHADER);
    GLint fragment_shader = load_shader("src/shaders/fragment.glsl", GL_FRAGMENT_SHADER);

    GLint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint vao = create_object();

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        const auto time = static_cast<float>(glfwGetTime());
        const float color = time - static_cast<int>(time);

        glClearColor(color, color, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Rendering
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glUseProgram(shader_program);
            glBindVertexArray(vao);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glfwTerminate();

    return
            0;
}
