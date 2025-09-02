//
// Created by leo on 8/8/25.
//

#ifndef PROGRAM_HPP
#define PROGRAM_HPP
#include "Material.hpp"


#include <filesystem>
#include <format>
#include <string>

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "glm/mat4x4.hpp"

namespace wrld {
    enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER };

    std::string get_type_name(ShaderType type);

    /// Combines a vertex and a fragment shader
    class Program {
    public:
        /// Loads a shader file in which both shaders (vertex & fragment)
        /// are defined (using #ifdef directives) to create the program.
        explicit Program(const std::string &combined_shader_path);

        /// Loads a vertex and a fragment shader file to create the program.
        Program(const std::string &vertex_shader_path, const std::string &fragment_shader_path);

        Program(Program &&other) noexcept;

        Program &operator=(Program &&other) noexcept;


        ~Program();

        /// Use this program in the GL context.
        void use() const;

        void set_uniform(const std::string &uniform, float value) const;

        void set_uniform(const std::string &uniform, int value) const;

        void set_uniform(const std::string &uniform, unsigned value) const;

        void set_uniform(const std::string &uniform, const glm::vec3 &value) const;

        void set_uniform(const std::string &uniform, const glm::vec4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat4x4 &value) const;

        void set_uniform(const std::string &uniform, const glm::mat3x3 &value) const;

        void set_uniform(const std::string &uniform, const Material &material) const;

    private:
        GLuint vertex_shader;
        GLuint fragment_shader;
        GLuint gl_program;

        static std::string read_file(const std::string &path);

        /// Preprocess the GLSL source code to fit our needs.
        /// We add a #define with the expected type of the shader. This allows to
        /// combine the whole pipeline into one .glsl file.
        static std::string preprocess_source(const std::string &shader_source, ShaderType shader_type);

        static void compile_shader(GLuint gl_shader, const std::string &shader_path, ShaderType type);
    };
} // namespace wrld

#endif // PROGRAM_HPP
