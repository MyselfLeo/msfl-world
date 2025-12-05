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
#include <utility>

#include <sstream>
#include <glm/gtc/type_ptr.inl>
#include <wrld/Main.hpp>

namespace wrld::rsc {
    std::string get_type_name(const ShaderType type) {
        switch (type) {
            case ShaderType::Vertex:
                return "VERTEX_SHADER";
            case ShaderType::Fragment:
                return "FRAGMENT_SHADER";
            case ShaderType::Compute:
                return "COMPUTE_SHADER";
            default:
                std::unreachable();
        }
    }

    GLenum get_gl_type(const ShaderType type) {
        switch (type) {
            case ShaderType::Vertex:
                return GL_VERTEX_SHADER;
            case ShaderType::Fragment:
                return GL_FRAGMENT_SHADER;
            case ShaderType::Compute:
                return GL_COMPUTE_SHADER;
            default:
                std::unreachable();
        }
    }

    // Just a way to use a 2-in-1 shader file without specifying the same path twice.
    Program::Program(std::string name, World &world) : Resource(std::move(name), world) {}

    Program &Program::from_default() {
        shader_source(ShaderType::Vertex, shader::DEFAULT_VERTEX);
        shader_source(ShaderType::Fragment, shader::DEFAULT_FRAGMENT);
        reload();
        return *this;
    }

    Program &Program::shader_path(const ShaderType shader_type, const std::string &path) {
        shader_paths[shader_type] = path;
        return *this;
    }

    Program &Program::shader_source(const ShaderType shader_type,
                                    const std::string &source) {
        shader_sources[shader_type] = source;
        return *this;
    }

    Program::~Program() {
        for (const auto &s: gl_shaders | std::views::values) {
            glDeleteShader(s);
        }
        glDeleteProgram(gl_program);
    }

    void Program::use() {
        if (!compiled_once) {
            reload();
        }
        glUseProgram(gl_program);
    }

    void Program::set_uniform(const std::string &uniform, const float value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniform1f(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const int value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniform1i(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const unsigned value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniform1ui(uniform_loc, value);
    }

    void Program::set_uniform(const std::string &uniform, const glm::vec3 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniform3f(uniform_loc, value[0], value[1], value[2]);
    }

    void Program::set_uniform(const std::string &uniform, const glm::vec4 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniform4f(uniform_loc, value[0], value[1], value[2], value[3]);
    }

    void Program::set_uniform(const std::string &uniform,
                              const glm::mat3x3 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
            return;
        }

        glUniformMatrix3fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Program::set_uniform(const std::string &uniform,
                              const Rc<Material> &material) const {
        // Constant colors
        set_uniform(uniform + ".diffuse_color", material->get_diffuse_color());
        set_uniform(uniform + ".specular_intensity", material->get_specular_intensity());

        // Diffuse map
        if (material->get_diffuse_map().has_value()) {
            material->get_diffuse_map().value()->use(0);

            set_uniform(uniform + ".use_diffuse", true);
            set_uniform(uniform + ".diffuse", 0);
        } else {
            set_uniform(uniform + ".use_diffuse", false);
        }

        // Specular map
        if (material->get_specular_map().has_value()) {
            material->get_specular_map().value()->use(1);

            set_uniform(uniform + ".use_specular", true);
            set_uniform(uniform + ".specular", 1);
        } else {
            set_uniform(uniform + ".use_specular", false);
        }

        set_uniform(uniform + ".shininess", material->get_shininess());

        set_uniform(uniform + ".use_mesh_color", material->is_using_mesh_color());
        set_uniform(uniform + ".do_lighting", material->is_doing_lighting());
    }

    void Program::reload() {
        wrldInfo(std::format("Recompiling shaders for program '{}'", get_name()));

        // Create the Program if required
        if (gl_program == 0) {
            gl_program = glCreateProgram();
            if (gl_program == 0) {
                throw std::runtime_error("Unable to create OpenGL program object");
            }
        }

        constexpr std::array SHADER_TYPES = {ShaderType::Vertex, ShaderType::Fragment,
                                             ShaderType::Compute};

        std::unordered_map<ShaderType, std::string> final_sources;

        // Compile each shader one by one
        for (const auto st: SHADER_TYPES) {
            std::string source;
            if (shader_paths.contains(st) && !shader_sources.contains(st)) {
                wrldInfo(std::format("\tCompiling {} from '{}'", get_type_name(st),
                                     shader_paths.at(st)));
                source = read_file(shader_paths.at(st));
            } else if (shader_sources.contains(st)) {
                wrldInfo(std::format("\tCompiling {} from source", get_type_name(st)));
                source = shader_sources.at(st);
            } else {
                continue;
            }

            if (!gl_shaders.contains(st)) {
                gl_shaders[st] = glCreateShader(get_gl_type(st));
                if (gl_shaders[st] == 0) {
                    throw std::runtime_error(
                            "Unable to create OpenGL vertex shader object");
                }
            }

            compile_shader(gl_shaders[st], preprocess_source(source, st));
            glAttachShader(gl_program, gl_shaders[st]);
        }

        glLinkProgram(gl_program);
        compiled_once = true;
    }

    void Program::set_uniform(const std::string &uniform,
                              const glm::mat4x4 &value) const {
        const GLint uniform_loc = glGetUniformLocation(gl_program, uniform.c_str());
        if (uniform_loc == -1) {
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

    std::string Program::preprocess_source(const std::string &shader_source,
                                           const ShaderType shader_type) {
        // We need to find the #version line. We'll remove it but re-add it later

        // Sadly we can't use the constexpr Main::get_platform() because it will still
        // try to parse std::regex_constants::multiline under MSVC
        std::regex re;
#if defined(_MSC_VER) && !defined(__clang__)
        re = std::regex(R"(^#version.*$)");
#else
        re = std::regex(R"(^#version.*$)", std::regex_constants::multiline);
#endif

        // Query the #version line
        std::smatch match;

        if (!std::regex_search(shader_source, match, re)) {
            throw std::runtime_error(
                    std::format("No #version directive found in the shader."));
        }

        if (match.size() > 1)
            throw std::runtime_error(
                    std::format("Multiple #version directives found in the shader."));

        // Remove this same line from the source
        std::string stripped_source = std::regex_replace(shader_source, re, "");

        // Build the processed source
        std::string res = match[0]; // The version line
        res.append("\n");
        res.append(std::format("#define {}\n", get_type_name(shader_type)));
        res.append(stripped_source);

        return res;
    }

    void Program::compile_shader(const GLuint gl_shader, const std::string &shader_src) {
        const char *shader_src_str = shader_src.c_str();
        glShaderSource(gl_shader, 1, &shader_src_str, nullptr);
        glCompileShader(gl_shader);

        // Check compilation error
        int success;
        glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);

        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(gl_shader, 512, nullptr, infoLog);
            throw std::runtime_error(
                    std::format("Failed to compile shader: {}", infoLog));
        }
    }
} // namespace wrld::rsc
