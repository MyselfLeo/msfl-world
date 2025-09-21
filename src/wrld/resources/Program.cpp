//
// Created by leo on 8/8/25.
//

#include <wrld/resources/Program.hpp>
#include <wrld/shaders/vertex/default_shader.hpp>
#include <wrld/shaders/fragment/default_shader.hpp>

#include <wrld/resources/Rc.hpp>
#include <wrld/logs.hpp>

#include <format>
#include <fstream>
#include <iostream>
#include <regex>

#include "glm/gtc/type_ptr.inl"

namespace wrld::rsc {
    std::string get_type_name(const ShaderType type) {
        switch (type) {
            case VERTEX_SHADER:
                return "VERTEX_SHADER";
            case FRAGMENT_SHADER:
                return "FRAGMENT_SHADER";
            default:
                throw std::runtime_error(std::format("Invalid shader type"));
        }
    }

    // Just a way to use a 2-in-1 shader file without specifying the same path twice.
    Program::Program(std::string name, World &world /*, Rc<Resource> *rc*/) :
        Resource(std::move(name), world /*, rc*/) {
        reload_from_source(shader::DEFAULT_VERTEX, shader::DEFAULT_FRAGMENT);
    }

    Program &Program::from_file(const std::string &combined_shader_path) {
        this->vertex_shader_path = combined_shader_path;
        this->fragment_shader_path = combined_shader_path;
        reload_from_file();
        return *this;
    }

    Program &Program::from_file(const std::string &vertex_path, const std::string &fragment_path) {
        this->vertex_shader_path = vertex_path;
        this->fragment_shader_path = fragment_path;
        reload_from_file();
        return *this;
    }

    Program &Program::from_source(const std::string &combined_shader_src) {
        reload_from_source(combined_shader_src, combined_shader_src);
        return *this;
    }

    Program &Program::from_source(const std::string &vertex_source, const std::string &fragment_source) {
        reload_from_source(vertex_source, fragment_source);
        return *this;
    }

    // Program::Program(Program &&other) noexcept :
    //     vertex_shader(other.vertex_shader), fragment_shader(other.fragment_shader), gl_program(other.gl_program) {
    //     other.vertex_shader = 0;
    //     other.fragment_shader = 0;
    //     other.gl_program = 0;
    // }
    //
    // Program &Program::operator=(Program &&other) noexcept {
    //     vertex_shader = other.vertex_shader;
    //     fragment_shader = other.fragment_shader;
    //     gl_program = other.gl_program;
    //     other.vertex_shader = 0;
    //     other.fragment_shader = 0;
    //     other.gl_program = 0;
    //     return *this;
    // }

    Program::~Program() {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(gl_program);
    }

    void Program::use() const { glUseProgram(gl_program); }

    void Program::set_uniform(const std::string &uniform, const float value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniform1f(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const int value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniform1i(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const unsigned value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniform1ui(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const glm::vec3 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniform3f(uniform_loc, value[0], value[1], value[2]);
    }

    void Program::set_uniform(const std::string &uniform, const glm::vec4 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniform4f(uniform_loc, value[0], value[1], value[2], value[3]);
    }

    void Program::set_uniform(const std::string &uniform, const glm::mat3x3 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniformMatrix3fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Program::set_uniform(const std::string &uniform, const Material &material) const {
        // Constant colors
        set_uniform(uniform + ".diffuse_color", material.get_diffuse_color());
        set_uniform(uniform + ".specular_intensity", material.get_specular_intensity());

        // Diffuse map
        if (material.get_diffuse_map().has_value()) {
            material.get_diffuse_map().value().get()->use(0);

            set_uniform(uniform + ".use_diffuse", true);
            set_uniform(uniform + ".diffuse", 0);
        } else {
            set_uniform(uniform + ".use_diffuse", false);
        }

        // Specular map
        if (material.get_specular_map().has_value()) {
            material.get_specular_map().value().get()->use(1);

            set_uniform(uniform + ".use_specular", true);
            set_uniform(uniform + ".specular", 1);
        } else {
            set_uniform(uniform + ".use_specular", false);
        }

        set_uniform(uniform + ".shininess", material.get_shininess());

        set_uniform(uniform + ".use_mesh_color", material.is_using_mesh_color());
        set_uniform(uniform + ".do_lighting", material.is_doing_lighting());
    }

    void Program::reload() const {
        wrldInfo("Reloading shaders...");
        // Compile the shaders, check for error
        compile_shader(vertex_shader, vertex_shader_path, VERTEX_SHADER);
        compile_shader(fragment_shader, fragment_shader_path, FRAGMENT_SHADER);

        // glAttachShader(gl_program, vertex_shader);
        // glAttachShader(gl_program, fragment_shader);
        // glLinkProgram(gl_program);
    }

    void Program::set_uniform(const std::string &uniform, const glm::mat4x4 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            // throw std::runtime_error(std::format("Undefined uniform '{}'", uniform));
            return;
        }

        glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    std::string Program::read_file(const std::string &path) {
        std::ifstream file(path);

        if (!file.is_open()) {
            throw std::runtime_error(std::format("Failed to load file {}", path));
        }

        // Load the file into a stringbuffer that we'll then convert to string
        std::stringstream buffer;
        buffer << file.rdbuf();

        file.close();
        return buffer.str();
    }

    void Program::reload_from_file() {
        if (vertex_shader == 0) {
            vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            if (vertex_shader == 0) {
                throw std::runtime_error("Unable to create OpenGL vertex shader object");
            }
        }

        if (fragment_shader == 0) {
            fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            if (fragment_shader == 0) {
                throw std::runtime_error("Unable to create OpenGL fragment shader object");
            }
        }

        // Compile the shaders, check for error
        wrldInfo(std::format("Loading shader {}", vertex_shader_path));
        const std::string vertex_src = read_file(vertex_shader_path);
        wrldInfo(std::format("Loading shader {}", fragment_shader_path));
        const std::string fragment_src = read_file(fragment_shader_path);

        compile_shader(vertex_shader, vertex_src, VERTEX_SHADER);
        compile_shader(fragment_shader, fragment_src, FRAGMENT_SHADER);

        // Create the program
        if (gl_program == 0) {
            gl_program = glCreateProgram();
            if (gl_program == 0) {
                throw std::runtime_error("Unable to create OpenGL program object");
            }

            glAttachShader(gl_program, vertex_shader);
            glAttachShader(gl_program, fragment_shader);
        }

        glLinkProgram(gl_program);

        compiled_once = true;
    }

    void Program::reload_from_source(const std::string &vertex_src, const std::string &fragment_src) {
        if (vertex_shader == 0) {
            vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            if (vertex_shader == 0) {
                throw std::runtime_error("Unable to create OpenGL vertex shader object");
            }
        }

        if (fragment_shader == 0) {
            fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            if (fragment_shader == 0) {
                throw std::runtime_error("Unable to create OpenGL fragment shader object");
            }
        }

        compile_shader(vertex_shader, vertex_src, VERTEX_SHADER);
        compile_shader(fragment_shader, fragment_src, FRAGMENT_SHADER);

        // Create the program
        if (gl_program == 0) {
            gl_program = glCreateProgram();
            if (gl_program == 0) {
                throw std::runtime_error("Unable to create OpenGL program object");
            }

            glAttachShader(gl_program, vertex_shader);
            glAttachShader(gl_program, fragment_shader);
        }

        glLinkProgram(gl_program);

        compiled_once = true;
    }

    std::string Program::preprocess_source(const std::string &shader_source, const ShaderType shader_type) {
        // We need to find the #version line. We'll remove it but re-add it later
        const std::regex re(R"(^#version.*$)", std::regex_constants::multiline);

        // Query the #version line
        std::smatch match;

        if (!std::regex_search(shader_source, match, re)) {
            throw std::runtime_error(std::format("No #version directive found in the shader."));
        }

        if (match.size() > 1)
            throw std::runtime_error(std::format("Multiple #version directives found in the shader."));

        // Remove this same line from the source
        std::string stripped_source = std::regex_replace(shader_source, re, "");

        // Build the processed source
        std::string res = match[0]; // The version line
        res.append("\n");
        res.append(std::format("#define {}\n", get_type_name(shader_type)));
        res.append(stripped_source);

        return res;
    }

    void Program::compile_shader(const GLuint gl_shader, const std::string &shader_src, const ShaderType type) {
        wrldInfo(std::format("Compiling shader {}", gl_shader));

        // const std::string shader_src = read_file(shader_path);

        const std::string preprocessed_src = preprocess_source(shader_src, type);

        const char *shader_src_str = preprocessed_src.c_str();
        glShaderSource(gl_shader, 1, &shader_src_str, nullptr);
        glCompileShader(gl_shader);

        // Check compilation error
        int success;
        glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(gl_shader, 512, nullptr, infoLog);
            throw std::runtime_error(std::format("Failed to compile shader: {}", infoLog));
        }
    }
} // namespace wrld::rsc
